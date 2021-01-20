#include "fd_forward.h"
#include "regionMapping.h"

dl_matrix3du_t* image_matrix;

esp_err_t capture_handler(uint8_t** out_buf) {
    camera_fb_t* fb = esp_camera_fb_get();

    if (!fb) {
      // Serial.println("Camera capture failed");
      return ESP_FAIL;
    }

    image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    if (!image_matrix) {
      esp_camera_fb_return(fb);
      // Serial.println("dl_matrix3du_alloc failed");
      return ESP_FAIL;
    }

    *out_buf = image_matrix->item;

    bool s = fmt2rgb888(fb->buf, fb->len, fb->format, *out_buf);
    esp_camera_fb_return(fb);
    if(!s){
      dl_matrix3du_free(image_matrix);
      // Serial.println("to rgb888 failed");
      return ESP_FAIL;
    }

    return ESP_OK;
}

void cameraStart() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  
  if (esp_camera_init(&config) != ESP_OK) {
    // Serial.printf("Camera init failed with error");
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  
  s->set_framesize(s, FRAMESIZE_QVGA);
  
  s->set_vflip(s, 0);
  s->set_hmirror(s, 0);

  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 1);
  s->set_ae_level(s, 2);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  
  s->set_wb_mode(s, 2);
  s->set_saturation(s, 2);
  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
}

uint32_t increaseColor(uint8_t r, uint8_t g, uint8_t b) {
  r = std::max(0, r - 60);
  r = std::sqrt(r / 255.0) * 255;
  g = std::sqrt(g / 255.0) * 255;
  b = std::sqrt(b / 255.0) * 255;
  
  const int cMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
  const int cMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
  const float chr = cMax - cMin;
  const float v = std::min(255.0, cMax * 1.75);
  float h = 0;

  if (v > 0) {
    if (chr > 0) {
      if (r == cMax) {
        h = (g - b) / chr;
        if (h < 0) h += 6;
      } 
      else if (g == cMax) {
        h = 2 + (b - r ) / chr;
      }
      else if (b == cMax) {
        h = 4 + (r - g) / chr;
      }
    }
  }

  const int i = h;
  const float f = h - i; 
  const float q = v * (1 - f);
  const float t = v * f;
  switch (i) {
    case 0: r = v; g = t; b = 0; break;
    case 1: r = q; g = v; b = 0; break;
    case 2: r = 0; g = v; b = t; break;
    case 3: r = 0; g = q; b = v; break;
    case 4: r = t; g = 0; b = v; break;
    default: r = v; g = 0; b = q;
  }

  return (r << 16) | (g << 8) | b;
}

uint32_t avgColorInRange(const std::pair<uint32_t, uint32_t> ranges[], int length, uint8_t* cameraBuf) {
  int r = 0, g = 0, b = 0, total = 0;
  for (int i = 0; i < length; i++) {
    pixelRanges range = ranges[i];
    for (int p = range.first; p <= range.second; p++) {
      int location = p * 3;
      r += cameraBuf[location] & 0xff;
      g += cameraBuf[location + 1] & 0xff;
      b += cameraBuf[location + 2] & 0xff;
      total++;
    }
  }

  r /= total;
  g /= total;
  b /= total;

  return increaseColor(r, g, b);
}

void seeColorsFromCamera(uint32_t* colors) {
  uint8_t* cameraBuf = NULL;
  if (ESP_OK != capture_handler(&cameraBuf)) {
    sendingColors = false;
    return;
  }

  for (int i = 0; i < 6; i++) {
    int length = 0;
    const pixelRanges* ranges = NULL;

    switch (i) {
      case 2:
        length = sizeof(map_LT);
        ranges = map_LT;
        break;
      case 1:
        length = sizeof(map_MT);
        ranges = map_MT;
        break;
      case 0:
        length = sizeof(map_RT);
        ranges = map_RT;
        break;
      case 5:
        length = sizeof(map_RB);
        ranges = map_RB;
        break;
      case 4:
        length = sizeof(map_MB);
        ranges = map_MB;
        break;
      case 3:
        length = sizeof(map_LB);
        ranges = map_LB;
        break;
    }

    length /= sizeof(pixelRanges);
    colors[i] = avgColorInRange(ranges, length, cameraBuf);
  }

  dl_matrix3du_free(image_matrix);
}

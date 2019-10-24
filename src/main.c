/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mgos.h"
#include "mgos_dht.h"
#include "mgos_rpc.h"
#include "mgos_ili9341.h"
#include "fonts/FreeMonoBold9pt7b.h"
#include "fonts/FreeMonoBold12pt7b.h"
#include "mgos_mqtt.h"
#include "mgos_gpio.h"
#include "stdlib.h"
#include <time.h>
#include "stdio.h"
#include "mgos_wifi.h"

time_t t;
long timeNow = 0;
long timeOpen = 0;
long timeClose = 0;
struct tm *p;
int year = 0, mon = 0, mday = 0;
int hour = 0, min = 0, sec = 0;
int timerId = 0;
int isOpen = 1;
float temperatureNow = 0;
bool isToOpen = false;
bool isToClose = false;
struct mgos_config_wifi_sta sta;
int isConnectWifi = 0;

// 上报设备状态
void sendStatus() {
  // 温度、时间、开关状态、定时开机、定时关机
  mgos_mqtt_pubf("/icebox/status", 0, false, "%.1lf,%ld,%ld,%ld,%ld", temperatureNow, timeNow, isOpen, timeOpen, timeClose);
}

// 每3s检测温度
static void timer_cb(void *dht) {
  float t = mgos_dht_get_temp(dht);
  LOG(LL_INFO, ("Temperature: %.1lf, ismqtt: %i,pin:%i", t, mgos_mqtt_global_is_connected() ? 1 :0,isOpen));
  if (temperatureNow != t)
  {
      char s[100];
      sprintf(s,"Temperature:%.1lf'C",t);
      mgos_ili9341_print(15, 110, s);
      temperatureNow = t;
  }
  // 偷懒，每3s直接上报状态
  sendStatus();
}

// 每秒计时及定时开关机简单测试
static void timer_per_sec() {
  if (timeNow > 0)
  {
    timeNow++;
    p = gmtime(&timeNow);
   year = 1900 + p->tm_year;
   mon = 1 + p->tm_mon;
   mday = p->tm_mday;

   hour = 8 + p->tm_hour; //获取当地时间，与UTC时间相差8小时   
   min = p->tm_min;
   sec = p->tm_sec;

  if (timeNow > 0 && timeOpen > 0 && timeNow < timeOpen)
   {
      isToOpen = true;
   }

   if (timeNow > 0 && timeClose > 0 && timeNow < timeClose)
   {
      isToClose = true;
   }

   if (timeNow > 0 && timeOpen > 0 && timeNow > timeOpen && isToOpen)
   {
      isToOpen = false;
      mgos_gpio_write(21, 1);
      isOpen = 1;
      sendStatus();
   }
   if (timeNow > 0 && timeClose > 0 && timeNow == timeClose)
   {
      isToClose = false;
      mgos_gpio_write(21, 0);
      isOpen = 0;
      sendStatus();
   }

   char s[100];
   sprintf(s,"%d-%.2d-%d %.2d:%.2d:%.2d  ",year, mon, mday, hour, min, sec);
   mgos_ili9341_print(15, 80, s);
   LOG(LL_INFO, ("%d-%d-%d %.2d:%.2d:%.2d", year, mon, mday,hour, min, sec));
  }
}

// 时间同步
static void subTimeHandler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud) {
      // LOG(LL_INFO, ("subHandler topic:%s, topic_len: %i, msg:%s, msg_len:%i",  topic, topic_len, msg, msg_len));
      timeNow = atol(msg);
      LOG(LL_INFO, ("subHandler time:%ld",  timeNow));
      if (timerId != 0)
      {
        mgos_clear_timer(timerId);
       }
     
     timerId = mgos_set_timer(1000, true, timer_per_sec, &timeNow);
}

// 开机
static void subOpenHandler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud) {
      LOG(LL_INFO, ("subOpenHandler"));
      mgos_gpio_write(21, 1);
      isOpen = 1;
      sendStatus();
}

// 关机
static void subCloseHandler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud) {
      LOG(LL_INFO, ("subCloseHandler"));
      mgos_gpio_write(21, 0);
      isOpen = 0;
      sendStatus();
}

// 设置定时开机，简单测试
static void subScheduleOpenHandler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud) {
      LOG(LL_INFO, ("subScheduleOpenHandler"));
      timeOpen = atol(msg);
      sendStatus();
}

// 设置定时关机，简单测试
static void subScheduleCloseHandler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud) {
      LOG(LL_INFO, ("subScheduleCloseHandler"));
      timeClose = atol(msg);
      sendStatus();
}

enum mgos_app_init_result mgos_app_init(void) {
  mgos_ili9341_spi_init();
  mgos_ili9341_drawDIF( 0, 0, "/t1.dif" );
  mgos_ili9341_set_bgcolor(0x5E, 0xCB, 0xF6);
  mgos_ili9341_set_fgcolor(0xff, 0xff, 0xff);  
  mgos_ili9341_set_font(&FreeMonoBold9pt7b);         // Set font
  char *topic_time = "/icebox/time";
  char *topic_open = "/icebox/open";
  char *topic_close = "/icebox/close";
  char *topic_schedule_open = "/icebox/schedule/open";
  char *topic_schedule_close = "/icebox/schedule/close";

  mgos_mqtt_sub(topic_time, subTimeHandler, NULL);
  mgos_mqtt_sub(topic_open, subOpenHandler, NULL);
  mgos_mqtt_sub(topic_close, subCloseHandler, NULL);
  mgos_mqtt_sub(topic_schedule_open, subScheduleOpenHandler, NULL);
  mgos_mqtt_sub(topic_schedule_close, subScheduleCloseHandler, NULL);

  mgos_gpio_set_mode(21, MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_set_pull(21, MGOS_GPIO_PULL_UP);
  mgos_gpio_setup_output(21, true);
  mgos_mqtt_global_connect();
  struct mgos_dht *dht = mgos_dht_create(mgos_sys_config_get_app_pin(), DHT11);
  mgos_set_timer(3000, true, timer_cb, dht);

  return MGOS_APP_INIT_SUCCESS;
}





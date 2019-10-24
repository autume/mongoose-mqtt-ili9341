## A demo Mongoose OS for mqtt and ili9341
### overview
This is an demo app. The relay on esp32 can be controlled via MQTT and the time and temperature are displayed on the screen.
### config_schema:
#### SPI and ili9341
the cs connect to TFT_CS
```
- ["spi.enable", true]
 - ["spi.cs1_gpio", -1]
 - ["spi.cs2_gpio", -1]
 - ["spi.mosi_gpio", 18]
 - ["spi.miso_gpio", 19]
 - ["spi.sclk_gpio", 5]
 - ["spi.cs0_gpio", 0]      # The ILI9341 CS pin
 - ["ili9341", "o", {title: "ILI9341 settings"}]
 - ["ili9341.cs_index", "i", 0, {title: "spi.cs*_gpio index, 0, 1 or 2"}]
 - ["ili9341.spi_freq", "i", 20000000, {title: "SPI frequency"}]
 - ["ili9341.dc_pin", "i", 33, {title: "TFT DC pin"}]
 - ["ili9341.rst_pin", "i", 15, {title: "RST pin. If set, will be used to reinit the display."}]
 - ["ili9341.width", "i", 320, {title: "TFT width in pixels"}]
 - ["ili9341.height", "i", 240, {title: "TFT height in pixels"}]
```
#### mqtt

```
- ["mqtt", "o", {title: "MQTT settings"}]
 - ["mqtt1", "mqtt", {title: "Backup MQTT settings"}]
 - ["mqtt.user", "s", "", {title: "User name"}]
 - ["mqtt.pass", "s", "", {title: "Password"}]
 - ["mqtt.server", "s", "suyaodong.top:1883", {title: "MQTT server"}]
 - ["mqtt.enable", "b", true, {title: "Enable MQTT"}]
 - ["mqtt.cloud_events", "b", true, {title: "Trigger cloud events when connected / disconnected"}]
```
#### wifi

```
- ["wifi.sta", "o", {title: "WiFi Station Config"}]
 - ["wifi.sta.enable", "b", true, {title: "Connect to existing WiFi"}]
 - ["wifi.sta.ssid", "s", "oden", {title: "SSID"}]
 - ["wifi.sta.pass", "s", "55558888", {title: "Password", type: "password"}]
 # sta1 and sta2 are exact copies of the above section and are used to support multiple station configurations.
 # - ["wifi.sta1", "wifi.sta", {title: "WiFi Station Config 1"}]
 - ["wifi.sta1.enable", false]
 - ["wifi.sta1.ssid", "s", "", {title: "SSID"}]
 - ["wifi.sta1.pass", "s", "", {title: "Password", type: "password"}]
 # - ["wifi.sta2", "wifi.sta", {title: "WiFi Station Config 2"}]
 - ["wifi.sta2.enable", false]
 - ["wifi.sta2.ssid", "s", "", {title: "SSID"}]
 - ["wifi.sta2.pass", "s", "", {title: "Password", type: "password"}]
 - ["wifi.sta_cfg_idx", "i", 0, {title: "WiFi station config index to use, 0, 1 or 2"}]
 - ["wifi.sta_connect_timeout", "i", 30, {title: "Timeout for connection, seconds"}]
```
![](./img1.jpg
)
![](./img2.jpg
)

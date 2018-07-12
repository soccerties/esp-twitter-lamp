# esp-twitter-lamp
ESP8266 based smart lamp which searches Twitter and flashes different colors depending on tweets.

## Lamp Usage
The lamp has 3 states
1. **Setup** 
With the built in LED *on* and the lamp lit *solid white* without fading. The lamp is in setup mode. The lamp will be acting as a Wifi access point transmitting SSID esp-lamp. After 3 minutes if the lamp has not been configured it will enter state #2.

2. **Unconfigured** 
With the built in LED *on* and the lamp *fading white* in and out. The lamp is in unconfigured mode. In this state the lamp is not connected to the Internet and is not searching twitter. Reset the lamp to enter state #1 so it can be configured with Internet access.

3. **Active** 
With the built in LED *off* and the lamp *fading different colors* in and out. The lamp is in active mode. In this mode the lamp is searching twitter periodically and flashing different colors depending on tweets.

On initial startup the lamp will attempt to connect to previously configured Wifi networks. If a known Wifi network is found. The lamp will connect and enter state #3 (active). 

On initial startup If no known Wifi networks are found. The lamp enters state #1 (setup) and can be setup by connecting to its SSID "esp-lamp". After 3 minutes in state #1 the lamp will enter state #2 (unconfigured). In order to leave state #2 the lamp must be restarted so it can be configured.

#### Wifi configuration

When the lamp is in state #1 you need to connect to its SSID and configure the Wifi network it can use for Internet access. After connecting to the esp-lamp SSID you will be prompted to "sign into network." If no prompt appears navigate to 192.168.4.1 with a web browser to configure. 

Follow the steps to configure the lamp's Wifi settings. If configuration is not working you can press the reset button twice within 5 seconds to clear Wifi settings and try again.

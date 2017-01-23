#include "rf24control.h"
#include "config.h"
#include "networking.h"

#define HEADER_MOTION 16 

// Address of our node and the other
const uint16_t this_node = RF24_NODEID;
const uint16_t other_node = 0;

RF24 radio(RF24_CE, RF24_CS);
RF24Network network(radio);

uint16_t cnt = 0;

struct out_payload
{
  uint32_t motion_detected_time;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t white;
  uint8_t mode;
};

out_payload my_out_payload;

void rf24Setup()
{
  radio.begin();
  network.begin( 90,  this_node);

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_16);
}

void rf24Loop()
{

  //we don't want to send anything, just be a relay
  network.update();
/*
  cnt++;
  if(cnt > 5000)
  {
    my_out_payload.motion_detected_time = 1212;
    my_out_payload.r = 12;
    my_out_payload.g = 12;
    my_out_payload.b = 12;
    my_out_payload.white = 12;
    my_out_payload.mode = 2;
    RF24NetworkHeader out_header(0, HEADER_MOTION);
    bool ok = network.write(out_header,&my_out_payload,sizeof(my_out_payload));
    if(ok)
    {
      mqttClient.publish("motion/rf24", "ok" );
    }
    else
    {
      mqttClient.publish("motion/rf24", "failed" );
    }
    cnt = 0;
  }
*/

}
// vim:ai:cin:sts=2 sw=2 ft=cpp

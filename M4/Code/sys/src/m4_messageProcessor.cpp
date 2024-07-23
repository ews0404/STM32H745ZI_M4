#include "../inc/m4_messageProcessor.h"
#include "../Common/inc/messageQueue.h"
#include "../system.h"
#include "../Common/inc/gpio.h"
#include "../Common/inc/messageID.h"

using namespace gpio;
namespace mq = messageQueue;


static mq::MessageQueueBufferType mbuf;

static void processMessage(mq::MessageQueueBufferType* msg);


void m4_messageProcessor::init(void)
{
}


void m4_messageProcessor::update(void)
{
	// if a message exists in the queue, copy it to the local buffer for processsing
	if (mq::hasMessages(mq::M7toM4)) {
		mq::readMessage(mq::M7toM4, &mbuf);
		processMessage(&mbuf);
	}
}


void processMessage(mq::MessageQueueBufferType* msg)
{
	switch (msg->messageID) {
		case(NoOp):
			break;
		
		case(SetLED):
			SYS_WARN("M4 does not support SetLED MessageID");
			break;
		
		case(PrintString):
			// just pretend the string will always be well formed (test code only)
			printf("%s\n", (char*)&msg->data);
			break;
		
		default:
			SYS_WARN("unrecognized messageID: %d", msg->messageID);
	}
}
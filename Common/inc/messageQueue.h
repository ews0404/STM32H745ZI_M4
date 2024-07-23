#pragma once
#include <stdint.h>
#include "hsem.h"
#include "messageID.h"

/* send messages between the two processor cores using FIFO queues with hardware semaphores to coordinate access */

#define MQ_MESSAGE_QUEUE_SIZE 8192						// 8kB queue going each direction
#define MQ_MAX_MESSAGE_SIZE 1536						// max message size, 1.5kB, also max Ethernet packet size	

namespace messageQueue
{
	// identify which direction each of the two FIFO queues moves data
	enum MessageQueueID {
		M4toM7 = 0,
		M7toM4 = 1
	};

	// defines an output buffer into which incoming messages get copied for processing
	struct MessageQueueBufferType {
		MessageID messageID;
		uint16_t dataLen;
		uint8_t data[MQ_MAX_MESSAGE_SIZE];
	} __attribute__((packed, aligned(4)));
	
	
	void init(MessageQueueID msgQueueID);
	bool hasMessages(MessageQueueID msgQueueID);
	void sendMessage(MessageQueueID msgQueueID, MessageID messageID, uint16_t dataLen, uint8_t* data);
	void readMessage(MessageQueueID msgQueueID, MessageQueueBufferType* buffer);
}
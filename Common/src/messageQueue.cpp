#include "../inc/messageQueue.h"
#include "../M4/Code/sys/system.h"
#include <string.h>

using namespace messageQueue;
using namespace hsem;

struct MessageQueue {
	uint32_t pendingMessages;				// the number of messages in the queue waiting to be processed
	uint32_t maxPendingMessages;			// the largest number of pending messages ever in the queue at once
	uint32_t bytesInQueue;					// number of bytes currently in the queue
	uint32_t maxBytesInQueue;				// the largest number of bytes ever contained in the queue
	uint32_t head;							// buffer index where the next byte should be written
	uint32_t tail;							// buffer index where the next byte should be read
	HSEM_ID hsemID;							// hardware semaphore controlling access to this queue
	uint8_t buffer[MQ_MESSAGE_QUEUE_SIZE];	// the queue data buffer
};


// Declare that we have a MessageQueue structure starting at the lowest address in the 32kB _sram4_IPC memory
// this way both M4 and M7 will accesss it at the same address. The _ssram4_IPC value is defined
// in the linker file for both processors. 
extern void* _sram4_mq;
static MessageQueue* mq = (MessageQueue*)&_sram4_mq;


static void writeBytes(MessageQueue* q, uint8_t* data, uint32_t dataLen);
static void readBytes(MessageQueue* q, uint8_t* dest, uint32_t dataLen);


void messageQueue::init(MessageQueueID msgQueueID)
{
	// do sketchy pointer math to get the memory location of our quasi-legal message queues,
	// msgQueueID is 0 for M4toM7, 1 for M7toM4
	MessageQueue* q = &mq[msgQueueID]; 
	
	// zero out all data, record which HSEM_ID should be locked to access this queue
	memset(q, 0, sizeof(MessageQueue));
	q->hsemID = (msgQueueID == M4toM7) ? hsemID_M4toM7 : hsemID_M7toM4;
}


bool messageQueue::hasMessages(MessageQueueID msgQueueID)
{
	// do sketchy pointer math to get the memory location of our quasi-legal message queues,
	// msgQueueID is 0 for M4toM7, 1 for M7toM4
	MessageQueue* q = &mq[msgQueueID]; 
	
	// this is a read-only operation so we do not need to acquire a lock
	return (q->pendingMessages > 0);
}


void messageQueue::sendMessage(MessageQueueID msgQueueID, MessageID command, uint16_t dataLen, uint8_t* data)
{
	// do sketchy pointer math to get the memory location of our quasi-legal message queues,
	// msgQueueID is 0 for M4toM7, 1 for M7toM4
	MessageQueue* q = &mq[msgQueueID]; 
	Core_ID coreID = (msgQueueID) ? m4_coreID : m7_coreID;
	
	// sanity checks
	uint32_t msgSize = (sizeof(command) + sizeof(dataLen) + dataLen);
	if (msgSize > MQ_MAX_MESSAGE_SIZE) { SYS_ERROR("message size too large"); }
	if ((MQ_MESSAGE_QUEUE_SIZE - q->bytesInQueue) < msgSize) { 
		// drop the message if there is no room for it, e.g. when one core is halted for debugging and not 
		// processing incoming messages 
		SYS_ERROR("message queue overflow, message dropped"); 
		return;
	}
	
	// spin wait until we acquire a hsem lock on the queue we want
	while (!lock(q->hsemID, coreID)) { }
	
	// write bytes into the queue
	writeBytes(q, (uint8_t*)&command, sizeof(MessageID));
	writeBytes(q, (uint8_t*)&dataLen, sizeof(dataLen));
	if (dataLen > 0) { writeBytes(q, data, dataLen); }
	
	// track the maximum number of bytes stored in the queue
	q->bytesInQueue += sizeof(MessageID) + sizeof(dataLen) + dataLen;
	if (q->bytesInQueue > q->maxBytesInQueue) { q->maxBytesInQueue = q->bytesInQueue; }
	
	// track the number of messages waiting to be read
	q->pendingMessages++;
	if (q->pendingMessages > q->maxPendingMessages) { q->maxPendingMessages = q->pendingMessages; }
	
	// unlock the queue hsem when done
	unlock(q->hsemID, coreID);
}


void messageQueue::readMessage(MessageQueueID msgQueueID, MessageQueueBufferType* buffer)
{
	// do sketchy pointer math to get the memory location of our quasi-legal message queues,
	// msgQueueID is 0 for M4toM7, 1 for M7toM4
	MessageQueue* q = &mq[msgQueueID]; 
	Core_ID coreID = (msgQueueID) ? m4_coreID : m7_coreID;
	
	if (q->pendingMessages == 0) {
		SYS_WARN("attempted to read empty message queue");
	} else {
		// sanity check
		uint32_t msgSize = sizeof(MessageID) + sizeof(buffer->dataLen) + buffer->dataLen;
		if (msgSize > q->bytesInQueue){ SYS_ERROR("message queue underflow"); }
		
		// spin wait until we acquire a hsem lock on the queue we want
		while (!lock(q->hsemID, coreID)) { }
		
		// read bytes into the data buffer
		readBytes(q, (uint8_t*)&buffer->messageID, sizeof(MessageID));
		readBytes(q, (uint8_t*)&buffer->dataLen, sizeof(buffer->dataLen));
		if (buffer->dataLen > 0) { readBytes(q, (uint8_t*)buffer->data, buffer->dataLen); }
		
		// track the number of bytes stored in the queue
		q->bytesInQueue -= msgSize;
		
		// track the number of messages waiting to be read
		q->pendingMessages--;
		
		// unlock the queue hsem when done
		unlock(q->hsemID, coreID);
	}
}


void writeBytes(MessageQueue* q, uint8_t* data, uint32_t dataLen)
{
	// copy the specified number of bytes from data souce into the message queue
	for (uint32_t i = 0; i < dataLen; ++i) {
		q->buffer[q->head] = data[i];
		q->head = ((q->head + 1) % MQ_MESSAGE_QUEUE_SIZE);
		if (q->head == q->tail) { SYS_ERROR("message queue overflow"); }
	}
}


void readBytes(MessageQueue* q, uint8_t* dest, uint32_t dataLen)
{
	// copy the specified number of bytes from the message queue to the specified data destination
	for (uint32_t i = 0; i < dataLen; ++i) {
		dest[i] = q->buffer[q->tail];
		q->tail = ((q->tail + 1) % MQ_MESSAGE_QUEUE_SIZE);
	}
}


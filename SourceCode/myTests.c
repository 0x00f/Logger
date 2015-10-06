#include "Unity/src/unity.h"
#include "main.h"
#include "string.h"

typedef struct {
	uint8_t buffer[5] __attribute__ ((aligned (32)));
	uint16_t w_index;
	bool isReady;
} TBuffer;

int a;
int b;

uint8_t src_array[10];
uint8_t dest_array[10];
uint8_t num_bytes;

TBuffer a_buff;
TBuffer b_buff;
TBuffer c_buff;

TBuffer *ACTIVE_TBUFFER;
TBuffer *PASSIVE_TBUFFER;

static void switch_active_buffer();

void setUp() {
	a = 1;
	b = 2;
	memset(src_array, 0xFF, 10);
	ACTIVE_TBUFFER = &a_buff;
	PASSIVE_TBUFFER = &b_buff;
}

void tearDown() {

}

void testWillAlwaysPass(void) {
	TEST_ASSERT_EQUAL_INT(42,42);
}

void testWillAlwaysFail(void) {
	TEST_ASSERT_EQUAL_INT(a,b);
}

void memcpyTest(void) {
	memcpy(dest_array, src_array, sizeof(dest_array));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(src_array, dest_array, 10);
}

void switchActiveBufferTest(void) {

	uint8_t temp_buffer[10];
	uint_fast8_t temp_windex = 0;

	a_buff.isReady = true;
	b_buff.isReady = true;
	c_buff.isReady = true;


//	ADC_DATA_CH[0] = (int16_t) 1;
//	ADC_DATA_CH[1] = (int16_t) 2;
//	ADC_DATA_CH[2] = (int16_t) 3;
//	ADC_DATA_CH[3] = (int16_t) 4;
//	ADC_DATA_CH[4] = (int16_t) -5;
//	ADC_DATA_CH[5] = (int16_t) -6;
//	ADC_DATA_CH[6] = (int16_t) -7;
//	ADC_DATA_CH[7] = (int16_t) -8;
//
	temp_buffer[0] = 1;
	temp_buffer[1] = 2;
	temp_buffer[2] = 3;
	temp_buffer[3] = 4;
	temp_buffer[4] = 5;
	temp_buffer[5] = 6;
	temp_buffer[6] = 7;
	temp_buffer[7] = 8;
	temp_buffer[8] = 9;
	temp_buffer[9] = 10;
//	temp_buffer[10] = 11;
//	temp_buffer[11] = 12;
//	temp_buffer[12] = 13;
//	temp_buffer[13] = 14;
//	temp_buffer[14] = 15;

	for(int i = 0; i<1;i++) {
		if((ACTIVE_TBUFFER->w_index + sizeof(temp_buffer)) > (sizeof(ACTIVE_TBUFFER->buffer))) {
			temp_windex = (sizeof(ACTIVE_TBUFFER->buffer)) - (ACTIVE_TBUFFER->w_index);
			//		sprintf(output_buf, "Buffer full, only writing %d Bytes\n", temp_windex);
			//		TM_USART_Puts(USART1, output_buf);
			//memcpy(&ACTIVE_BUFFER->[a_w_index], temp_buffer, sizeof(temp_buffer));
			/* Only copy enough bytes to fill the current buffer */
			memcpy(&ACTIVE_TBUFFER->buffer[ACTIVE_TBUFFER->w_index], temp_buffer, temp_windex);
			switch_active_buffer();
			//a_w_index = 0;
			//ACTIVE_BUFFER->w_index = 0;
			//transfer_data_flag = true;
			/* Copy remaining data to the new active buffer */
			//		sprintf(output_buf, "Switched buffer, remaining %d Bytes written\n", sizeof(temp_buffer) - temp_windex);
			//		TM_USART_Puts(USART1, output_buf);
			memcpy(&ACTIVE_TBUFFER->buffer[ACTIVE_TBUFFER->w_index], &temp_buffer[temp_windex], sizeof(temp_buffer) - temp_windex);
			ACTIVE_TBUFFER->w_index += sizeof(temp_buffer) - temp_windex;
		}
		/* We can fit all new data, just write to buffer */
		else {
			//memcpy(&ACTIVE_BUFFER->buffer[a_w_index], temp_buffer, sizeof(temp_buffer));
			memcpy(&ACTIVE_TBUFFER->buffer[ACTIVE_TBUFFER->w_index], temp_buffer, sizeof(temp_buffer));
			//a_w_index += 21;
			ACTIVE_TBUFFER->w_index += sizeof(temp_buffer);
		}

	}


}

static void switch_active_buffer() {

	if (ACTIVE_TBUFFER == &a_buff) {
		/* buffer A is now full, needs processing */
		a_buff.isReady = false;
		if(b_buff.isReady) {
			ACTIVE_TBUFFER = &b_buff;
			PASSIVE_TBUFFER = &a_buff;
		}
		else {
			ACTIVE_TBUFFER = &c_buff;
			PASSIVE_TBUFFER = &a_buff;
		}
	}

	else if(ACTIVE_TBUFFER == &b_buff) {
		/* buffer B is now full, needs processing */
		b_buff.isReady = false;
		if(c_buff.isReady) {
			ACTIVE_TBUFFER = &c_buff;
			PASSIVE_TBUFFER = &b_buff;
		}
		else {
			ACTIVE_TBUFFER = &a_buff;
			PASSIVE_TBUFFER = &b_buff;
		}
	}

//	if (ACTIVE_BUFFER == &a_buff) {
//		/* buffer A is now full, needs processing */
//		a_buff.isReady = false;
//		if(b_buff.isReady) {
//			ACTIVE_BUFFER = &b_buff;
//			PASSIVE_BUFFER = &a_buff;
//		}
//		else {
//			ACTIVE_BUFFER = &c_buff;
//			PASSIVE_BUFFER = &a_buff;
//		}
//	}
//
//	else if(ACTIVE_BUFFER == &b_buff) {
//		/* buffer B is now full, needs processing */
//		b_buff.isReady = false;
//		if(c_buff.isReady) {
//			ACTIVE_BUFFER = &c_buff;
//			PASSIVE_BUFFER = &b_buff;
//		}
//		else {
//			ACTIVE_BUFFER = &a_buff;
//			PASSIVE_BUFFER = &b_buff;
//		}
//	}
//
//	else if(ACTIVE_BUFFER == &c_buff) {
//		/* buffer B is now full, needs processing */
//		c_buff.isReady = false;
//		if(a_buff.isReady) {
//			ACTIVE_BUFFER = &a_buff;
//			PASSIVE_BUFFER = &c_buff;
//		}
//		else {
//			ACTIVE_BUFFER = &b_buff;
//			PASSIVE_BUFFER = &c_buff;
//		}
//	}

	}

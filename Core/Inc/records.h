/*
 * records.h
 *
 *  Created on: 29 черв. 2023 р.
 *      Author: 2andn
 */

#ifndef INC_RECORDS_H_
#define INC_RECORDS_H_
#include <cstdint>

#define ELEMENTS_SIZE (60 * 24)

class record {
private:
	uint16_t *elements;
	uint16_t indexWrite;
	uint16_t indexRead;
	uint16_t writeSize;
	uint16_t elementsLen;
public:
	uint16_t getValByIndex(uint16_t index);
	void put(uint16_t element);
	uint16_t get();
	uint16_t getSizeWrites();
	record(uint16_t *buffer, uint16_t len);
};

#endif /* INC_RECORDS_H_ */

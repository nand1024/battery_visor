/*
 * records.cpp
 *
 *  Created on: 29 черв. 2023 р.
 *      Author: 2andn
 */

#include <cstdint>
#include "records.h"

uint16_t record::getValByIndex(uint16_t index)
{
	uint16_t ind;
	if (index > elementsLen || index > writeSize) {
		return 65535;//err value
	}
	if (elementsLen > writeSize) {
		ind = index + 1;
	} else {
		ind = indexWrite + 1 + index;
		if (ind >= writeSize) {
			ind -= writeSize;
		}
	}
	return elements[ind];
}

void record::put(uint16_t val)
{
	if (++indexWrite == elementsLen) {
		indexWrite = 0;
	}
	elements[indexWrite] = val;
	if(writeSize < elementsLen)writeSize++;
}

uint16_t record::get()
{
	if(writeSize > 0) {
		writeSize--;
		if(++indexRead == elementsLen) {
			indexRead = 0;
		}
	} else {
		return 65535;
	}
	return elements[indexRead];
}

uint16_t record::getSizeWrites()
{
	return writeSize;
}

record::record(uint16_t *buffer, uint16_t len)
{
	elements = buffer;
	indexWrite = 0;
	indexRead = 0;
	writeSize = 0;
	elementsLen = len;
}

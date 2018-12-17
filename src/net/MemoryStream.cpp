#include "MemoryStream.h"
#include <cstdlib>
#include <algorithm> // std::max
#include <cassert>

void OutputMemoryStream::Write(const void *inData, size_t inByteCount)
{
	// make sure we have space
	const uint32_t resultHead = mHead + static_cast<uint32_t>(inByteCount);
	if (resultHead > mCapacity)
	{
		ReallocBuffer(std::max(mCapacity * 2, resultHead));
	}

	// Copy into buffer at head
	std::memcpy(mBuffer + mHead, inData, inByteCount);

	// Increment head for next write
	mHead = resultHead;
}

void OutputMemoryStream::ReallocBuffer(uint32_t inNewLength)
{
	mBuffer = static_cast<char*>(std::realloc(mBuffer, inNewLength));
	assert(mBuffer != nullptr && "OutputMemoryStream::ReallocBuffer() - std::realloc() failed.");
	mCapacity = inNewLength;
}

void InputMemoryStream::Read(void *outData, size_t inByteCount)
{
	uint32_t resultHead = mHead + static_cast<uint32_t>(inByteCount);
	assert(resultHead <= mCapacity && "InputMemoryStream::Read() - trying to read more data than available.");
	std::memcpy(outData, mBuffer + mHead, inByteCount);
	mHead = resultHead;
}

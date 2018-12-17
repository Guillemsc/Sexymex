#ifndef MEMORY_STREAM_H
#define MEMORY_STREAM_H

#include "ByteSwap.h"
#include <cstdint>
#include <vector>

enum class Endianness {
	LittleEndian,
	BigEndian
};

constexpr Endianness STREAM_ENDIANNESS = Endianness::BigEndian;
constexpr Endianness PLATFORM_ENDIANNESS = Endianness::LittleEndian;

// Default size set to the minimum MSS (MTU - IP and TCP header sizes)
// Typical MTU over Ethernet is 1500 bytes
// Minimum IP and TCP header sizes are 20 bytes each
constexpr uint32_t DEFAULT_STREAM_SIZE = 1460;

class OutputMemoryStream
{
public:

	// Constructor
	OutputMemoryStream(uint32_t inSize = DEFAULT_STREAM_SIZE):
		mBuffer(nullptr), mCapacity(0), mHead(0)
	{ ReallocBuffer(inSize); }

	// Destructor
	~OutputMemoryStream()
	{ std::free(mBuffer); }

	// Get pointer to the data in the stream
	const char *GetBufferPtr() const { return mBuffer; }
	uint32_t GetCapacity() const { return mCapacity; }
	uint32_t GetSize() const { return mHead; }

	// Clear the stream state
	void Clear() { mHead = 0; }

	// Write method
	void Write(const void *inData, size_t inByteCount);

	// Generic write for arithmetic types
	template< typename T >
	void Write( T inData )
	{
		static_assert(
				std::is_arithmetic< T >::value ||
				std::is_enum< T >::value,
				"Generic Write only supports primitive data types" );

		if( STREAM_ENDIANNESS == PLATFORM_ENDIANNESS )
		{
			Write( &inData, sizeof( inData ) );
		}
		else
		{
			T swappedData = ByteSwap( inData );
			Write( &swappedData, sizeof( swappedData ) );
		}
	}

	// Generic write for vectors of arithmetic types
	template< typename T >
	void Write( const std::vector< T >& inVector )
	{
		uint32_t elementCount = static_cast<uint32_t>(inVector.size());
		Write( elementCount );
		for( const T& element : inVector )
		{
			Write( element );
		}
	}

	// Write for strings
	void Write( const std::string& inString )
	{
		uint32_t elementCount = static_cast<uint32_t>(inString.size());
		Write( elementCount );
		Write( inString.data(), elementCount * sizeof( char ) );
	}
	

private:

	// Resize the buffer
	void ReallocBuffer(uint32_t inNewLength);

	char *mBuffer;
	uint32_t mCapacity;
	uint32_t mHead;
};

class InputMemoryStream
{
public:

	// Constructor
	InputMemoryStream(uint32_t inSize = DEFAULT_STREAM_SIZE) :
		mBuffer(static_cast<char*>(std::malloc(inSize))), mCapacity(inSize), mHead(0)
	{ }

	// Destructor
	~InputMemoryStream()
	{ std::free(mBuffer); }

	// Get pointer to the data in the stream
	char *GetBufferPtr() const { return mBuffer; }
	uint32_t GetCapacity() const { return mCapacity; }
	uint32_t GetSize() const { return mHead; }

	// Clear the stream state
	void Clear() { mHead = 0; }

	// Read method
	void Read(void *outData, size_t inByteCount);

	// Generic read for arithmetic types
	template< typename T >
	void Read( T& outData )
	{
		static_assert(
				std::is_arithmetic< T >::value ||
				std::is_enum< T >::value,
				"Generic Read only supports primitive data types" );

		if( STREAM_ENDIANNESS == PLATFORM_ENDIANNESS )
		{
			Read( &outData, sizeof( outData ) );
		}
		else
		{
			T unswappedData;
			Read( &unswappedData, sizeof( unswappedData ) );
			outData = ByteSwap(unswappedData);
		}
	}

	// Generic read for vectors of arithmetic types
	template< typename T >
	void Read( std::vector< T >& outVector )
	{
		uint32_t elementCount;
		Read( elementCount );
		outVector.resize( elementCount );
		for( const T& element : outVector )
		{
			Read( element );
		}
	}

	// Read for strings
	void Read( std::string& inString )
	{
		uint32_t elementCount;
		Read( elementCount );
		inString.resize(elementCount);
		for (auto &character : inString) {
			Read(character);
		}
	}

private:

	char *mBuffer;
	uint32_t mCapacity;
	uint32_t mHead;
};

#endif // MEMORY_STREAM_H

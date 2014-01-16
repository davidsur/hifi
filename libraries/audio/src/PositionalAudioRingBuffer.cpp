//
//  PositionalAudioRingBuffer.cpp
//  hifi
//
//  Created by Stephen Birarda on 6/5/13.
//  Copyright (c) 2013 HighFidelity, Inc. All rights reserved.
//

#include <cstring>

#include <Node.h>
#include <PacketHeaders.h>
#include <UUID.h>

#include "PositionalAudioRingBuffer.h"

#ifdef _WIN32
int isnan(double value) { return _isnan(value); }
#else
int isnan(double value) { return std::isnan(value); }
#endif

PositionalAudioRingBuffer::PositionalAudioRingBuffer(PositionalAudioRingBuffer::Type type) :
    AudioRingBuffer(NETWORK_BUFFER_LENGTH_SAMPLES_PER_CHANNEL),
    _type(type),
    _position(0.0f, 0.0f, 0.0f),
    _orientation(0.0f, 0.0f, 0.0f, 0.0f),
    _willBeAddedToMix(false),
    _shouldLoopbackForNode(false),
    _shouldOutputStarveDebug(true)
{

}

PositionalAudioRingBuffer::~PositionalAudioRingBuffer() {
}

int PositionalAudioRingBuffer::parseData(unsigned char* sourceBuffer, int numBytes) {
    unsigned char* currentBuffer = sourceBuffer + numBytesForPacketHeader(sourceBuffer);
    currentBuffer += NUM_BYTES_RFC4122_UUID; // the source UUID
    currentBuffer += parsePositionalData(currentBuffer, numBytes - (currentBuffer - sourceBuffer));
    currentBuffer += writeData((char*) currentBuffer, numBytes - (currentBuffer - sourceBuffer));

    return currentBuffer - sourceBuffer;
}

int PositionalAudioRingBuffer::parsePositionalData(unsigned char* sourceBuffer, int numBytes) {
    unsigned char* currentBuffer = sourceBuffer;

    memcpy(&_position, currentBuffer, sizeof(_position));
    currentBuffer += sizeof(_position);

    memcpy(&_orientation, currentBuffer, sizeof(_orientation));
    currentBuffer += sizeof(_orientation);

    // if this node sent us a NaN for first float in orientation then don't consider this good audio and bail
    if (isnan(_orientation.x)) {
        reset();
        return 0;
    }

    return currentBuffer - sourceBuffer;
}

bool PositionalAudioRingBuffer::shouldBeAddedToMix(int numJitterBufferSamples) {
    if (!isNotStarvedOrHasMinimumSamples(NETWORK_BUFFER_LENGTH_SAMPLES_PER_CHANNEL + numJitterBufferSamples)) {
        if (_shouldOutputStarveDebug) {
            qDebug() << "Starved and do not have minimum samples to start. Buffer held back.";
            _shouldOutputStarveDebug = false;
        }
        
        return false;
    } else if (samplesAvailable() < NETWORK_BUFFER_LENGTH_SAMPLES_PER_CHANNEL) {
        qDebug() << "Do not have number of samples needed for interval. Buffer starved.";
        _isStarved = true;
        
        // reset our _shouldOutputStarveDebug to true so the next is printed
        _shouldOutputStarveDebug = true;
        
        return false;
    } else {
        // good buffer, add this to the mix
        _isStarved = false;

        // since we've read data from ring buffer at least once - we've started
        _hasStarted = true;

        return true;
    }

    return false;
}

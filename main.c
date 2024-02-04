#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>


int main() {
enum EventName {

VoiceNoteOff = 0x80,
VoiceNoteOn = 0x90,
VoiceAftertouch = 0xA0,
VoiceControlChange = 0xB0,
VoiceProgramChange = 0xC0,
VoiceChannelPressure = 0xD0,
VoicePitchBend = 0xE0,
SystemExclusive = 0xF0,

};
enum MetaEventName {

MetaSequence = 0x00,
MetaText = 0x01,
MetaCopyright = 0x02,
MetaTrackName = 0x03,
MetaInstrumentName = 0x04,
MetaLyrics = 0x05,
MetaMarker = 0x06,
MetaCuePoint = 0x07,
MetaProgramName = 0x08,
MetaDeviceName = 0x09,
MetaChannelPrefix = 0x20,
MetaEndOfTrack = 0x2F,
MetaSetTempo = 0x51,
MetaSMPTEOffset = 0x54,
MetaTimeSignature = 0x58,
MetaKeySignature = 0x59,
MetaSequencerSpecific = 0x7F
};

FILE * fPointer;

freopen("output.txt","w",stdout); //remove this line to print to console instead of a text document

//Read header section

fPointer = fopen ("open.mid", "rb"); //change this to the midi file name and directory to be read
if  (fPointer == NULL) {
    printf("fPointer returned null!\nplease place your midi file in the same directory as 'MIDI Reader.exe' and name it open.mid\n\n");
    return 2;
}


//swaps 32 bit integer byte order
uint32_t swap32(uint32_t n) {
return ((n & 0x000000FF) << 24) |
      ((n & 0x0000FF00) <<  8) |
      ((n & 0x00FF0000) >>  8) |
      ((n & 0xFF000000) >> 24);
};
//Swaps 16 bit integer byte order
uint16_t swap16(uint16_t n) {
return ((n >> 8) | (n << 8));
};

char* readString(uint32_t nLength) {
    char* str = (char*)malloc(sizeof(char) * (nLength +1));
    if (str == NULL) {
        perror("Memory allocation failed");
        exit(1);
    }
        fread(str, sizeof(char), nLength, fPointer);

    return str;
}




uint32_t readValue() {

    uint8_t nByte = 0; //temporarily holds the byte to add to nValue
    uint32_t nValue = 0; //holds initial byte

    fread(&nValue, 1, 1, fPointer);
    if (nValue & 0x80) { // checks if the most significant bit is set
        nValue &= 0x7f; //extracts the first 7 bits
        do { //loop to read shift and add bits to nValue if the most significant bit is still set
            fread(&nByte, 1, 1, fPointer);
            nValue = (nValue << 7) | (nByte & 0x7f);
        } while (nByte & 0x80);

    }

return nValue;

}
//read header section

//read file ID and check
char* fileID = readString(4);
int fileIDCheck = strcmp(fileID, "MThd");
if (fileIDCheck == 0) {
    printf("file ID is valid \n");
}
else {
    printf("file ID is INVALID! \nit is possible the file loaded is corrupted or not a valid MIDI file at all\n\n");
    return 1;
}
//read header length
uint32_t headerLength;
fread(&headerLength, 4, 1, fPointer);
headerLength = swap32(headerLength);

printf("header length = %d bytes \n", headerLength);


//read midi file format
uint16_t fileFormat;
fread(&fileFormat, 2, 1, fPointer);
fileFormat = swap16(fileFormat);
if (fileFormat == 0) {
printf("file format = %d 'the file contains a single multi-channel track' \n", fileFormat);
}
else if (fileFormat == 1) { printf("file format = %d 'the file contains one or more simultaneous tracks (or MIDI outputs) of a sequence \n", fileFormat);
}
else if (fileFormat == 2) {
 printf("file format = %d 'the file contains one or more sequentially independent single-track patterns' \n", fileFormat);
}
else {
    printf("file format unrecognized?");
};
//read track chunks
uint16_t trackChunks;
uint16_t trackChunksCounter;
fread(&trackChunks, 2, 1, fPointer);
trackChunks = swap16(trackChunks);
printf("track chunks in file = %u \n", trackChunks);
//read division
int8_t division1;
uint8_t division2;
uint16_t division;
fread(&division1, 1, 1, fPointer);
if (division1 & 0x80) {
    printf("bit 7 of division1 is set \n");
    fread(&division2, 1, 1, fPointer);
    printf("%negative SMPTE is %d and ticks per frame is set to %d", division1, division2); //figure out how to use these values later
}
else {

    fread(&division, 1, 1, fPointer);
    division = (division >> 8);
    division = (division | division1);
    division = swap16(division);
 printf("bit 15 of divisionCheck is NOT set therefore the bits 14-0 represent the delta time per quarter note \n\nthe delta time per quarter note is %d \n", division);

};

//read track chunks section
while (trackChunksCounter < trackChunks) {

char* chunkID = readString(4);
printf("reading chunk '%s' \n",chunkID);
uint32_t trackLength;
fread(&trackLength, 4, 1, fPointer);
trackLength = swap32(trackLength);
printf("track length = %u \n", trackLength);



uint32_t nStatusTimeDelta;
uint8_t nStatus;
uint8_t nPreviousStatus;
bool trackEnd = false;

while(trackEnd == false) {


//read delta time
nStatusTimeDelta = readValue();
printf("delta time = %u \n", nStatusTimeDelta);

fread(&nStatus, 1, 1, fPointer); //read status byte


if (nStatus < 0x80) {
    printf("RUNNING STATUS USED\n");
    nStatus = nPreviousStatus;
    fseek(fPointer, -1, SEEK_CUR);
}

//read data section
else if ((nStatus & 0xF0) == VoiceNoteOff) {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0xF;
uint8_t nNoteID;
fread(&nNoteID, 1, 1, fPointer);
uint8_t nNoteVelocity;
fread(&nNoteVelocity, 1, 1, fPointer);
printf("voice note on/off message received.\n channel = %u \nnote ID = %u\nnote velocity = %u\n\n", nChannel, nNoteID, nNoteVelocity);
}

else if ((nStatus & 0xF0) == VoiceNoteOn) {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0xF;
uint8_t nNoteID;
fread(&nNoteID, 1, 1, fPointer);
uint8_t nNoteVelocity;
fread(&nNoteVelocity, 1, 1, fPointer);
printf("voice note on/off message received.\n channel = %u \nnote ID = %u\nnote velocity = %u\n\n", nChannel, nNoteID, nNoteVelocity);
}
else if ((nStatus & 0xF0) == VoiceAftertouch) {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0xF;
uint8_t nNoteID;
fread(&nNoteID, 1, 1, fPointer);
uint8_t nNoteVelocity;
fread(&nNoteVelocity, 1, 1, fPointer);
printf("voice aftertouch message received.\n channel = %u \nnote ID = %u\nnote velocity = %u\n\n", nChannel, nNoteID, nNoteVelocity);
}
else if ((nStatus & 0xF0) == VoiceControlChange) {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0xF;
uint8_t nNoteID; //controller number
fread(&nNoteID, 1, 1, fPointer);
uint8_t nNoteVelocity; //controller value
fread(&nNoteVelocity, 1, 1, fPointer);
printf("control change message received.\n channel = %u \ncontroller number = %u\ncontroller value = %u\n\n", nChannel, nNoteID, nNoteVelocity);
}
else if ((nStatus & 0xF0) == VoiceProgramChange) {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0x0F;
uint8_t nProgramID;
fread(&nProgramID, 1, 1, fPointer);
printf("program change message received.\n channel = %u\nprogram ID = %u\n\n", nChannel, nProgramID);
}
else if ((nStatus & 0xF0) == VoiceChannelPressure)  {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0x0F;
uint8_t nPressure;
fread(&nPressure, 1, 1, fPointer);
printf("channel pressure message received.\n channel = %u\pressure = %u\n\n", nChannel, nPressure);
}
else if ((nStatus & 0xF0) == VoicePitchBend) {
nPreviousStatus = nStatus;
uint8_t nChannel = nStatus & 0x0F;
uint8_t NLSB;
uint8_t NMSB;
fread(&NLSB, 1, 1, fPointer);
fread(&NMSB, 1, 1, fPointer);
printf("pitch bend message recieved.\n channel = %u\n least significant 7 bits = %u\n most significant 7 bits = %u\n\n", nChannel, NLSB, NMSB);
}
else if ((nStatus & 0xF0) == SystemExclusive) {


if (nStatus == 0xF0) {
        printf("system exclusive message begin.\nskipping message...\n");
fseek(fPointer, readValue(), SEEK_CUR); //skips reading the system exclusive
}
if (nStatus == 0xF7) {
        printf("system exclusive message begin.\nskipping message...\n");
fseek(fPointer, readValue(), SEEK_CUR);

}
if (nStatus == 0xFF) {
        printf("meta message begin.\n");

uint8_t nType;
fread(&nType, 1, 1, fPointer);
uint8_t nLength = readValue();
uint32_t nTempo;
uint8_t SMPTEHourByte; //contains framerate info as well
uint8_t SMPTEMinuteByte;
uint8_t SMPTESecondByte;
uint8_t SMPTEFrameByte;
uint8_t SMPTESubFrameByte;
uint8_t timeSigNumerator;
uint8_t timeSigDenominator;
uint8_t timeSigMetronome;
uint8_t timeSig32ndPerBeat;
int8_t keySigKey;
uint8_t keySigType;

switch (nType)
    {

    case MetaText:

        printf("meta text event read\n[%s]\n\n",readString(nLength));
        break;

    case MetaCopyright:
        printf("meta copyright read\n[%s]\n\n", readString(nLength));
        break;

    case MetaTrackName:
        printf("meta track name read\n[%s]\n\n", readString(nLength));
        break;
    case MetaInstrumentName:
        printf("meta instrument name read\n[%s]\n\n", readString(nLength));
        break;
    case MetaLyrics:
        printf("meta lyrics read\n[%s]\n\n", readString(nLength));
        break;

    case MetaMarker:
        printf("meta marker read\n[%s]\n\n", readString(nLength));
        break;
    case MetaCuePoint:
        printf("meta cue point read\n[%s]\n\n", readString(nLength));
        break;
    case MetaProgramName:
        printf("meta program name read\n[%s]\n\n", readString(nLength));
        break;
    case MetaDeviceName:
        printf("meta device name read\n[%s]\n\n", readString(nLength));
        break;
    case MetaChannelPrefix:
        printf("meta channel prefix read\n[CHANNEL %u\n\n", readString(nLength));
        break;
    case MetaEndOfTrack:
        printf("meta end of track... setting track end variable to true and incrementing trackChunkCounter variable\n\n");
        trackEnd = true;
        trackChunksCounter++;
        break;
    case MetaSetTempo:
        fread(&nTempo, 1, nLength, fPointer);
        printf("meta set tempo read\n[TEMPO = %u]\n\n", nTempo);
        break;
    case MetaSMPTEOffset:
        fread(&SMPTEHourByte, 1, 1, fPointer);
        printf("meta SMPTE offset read\n");
        if (SMPTEHourByte & 0x60) {
            printf ("[FRAMERATE = 30]\n");
            }
        else if (SMPTEHourByte & 0x40) {
            printf("[FRAMERATE = 29.97]\n");
            }
        else if (SMPTEHourByte & 0x20) {
            printf("[FRAMERATE = 25]\n");
            }
        else {
            printf("[FRAMERATE = 24]\n");
        }
        printf("[HOURS = %u]\n", SMPTEHourByte & 0x1F);
        fread(&SMPTEMinuteByte, 1, 1, fPointer);
        fread(&SMPTESecondByte, 1, 1, fPointer);
        fread(&SMPTEFrameByte, 1, 1, fPointer);
        fread(&SMPTESubFrameByte, 1, 1, fPointer);
        printf("[MINUTES = %u]\n[SECONDS = %u]\n[FRAMES = %u]\n[SUBFRAMES = %u]\n\n", SMPTEMinuteByte, SMPTESecondByte, SMPTEFrameByte, SMPTESubFrameByte);
        break;

    case MetaTimeSignature:
        fread(&timeSigNumerator, 1, 1, fPointer);
        fread(&timeSigDenominator, 1, 1, fPointer);
        fread(&timeSigMetronome, 1, 1, fPointer);
        fread(&timeSig32ndPerBeat, 1, 1, fPointer);
        timeSigDenominator = pow(2, timeSigDenominator);
        printf("meta time signature read\n[TIME SIGNATURE = %u/%u]\n[METRONOME CLOCKS PER TICK =%u]\n[32ND NOTES PER BEAT = %u]\n\n", timeSigNumerator, timeSigDenominator, timeSigMetronome, timeSig32ndPerBeat);
        break;


    case MetaKeySignature:
        fread(&keySigKey, 1, 1, fPointer);
        fread(&keySigType, 1, 1, fPointer);
        printf("meta key signature read\n");
        if (keySigType == 0) {
            switch (keySigKey)
            {
            case 7:
                printf("[KEY = C# Major]\n");
                break;
            case 6:
                printf("[KEY = F# Major]\n\n");
                break;
            case 5:
                printf("[KEY = B Major]\n\n");
                break;
            case 4:
                printf("[KEY = E Major]\n\n");
                break;
             case 3:
                printf("[KEY = A Major]\n\n");
                break;
            case 2:
                printf("[KEY = D Major]\n\n]");
                break;
            case 1:
                printf("[KEY = G Major]\n\n");
                break;
            case 0:
                printf("[KEY = C Major]\n\n");
                break;
            case -1:
                printf("[KEY = F Major]\n\n");
                break;
             case -2:
                printf("[KEY = Bb Major]\n\n");
                break;
            case -3:
                printf("[KEY = Eb Major]\n\n");
                break;
            case -4:
                printf("[KEY = Ab Major]\n\n");
                break;
            case -5:
                printf("[KEY = Db Major]\n\n");
                break;
            case -6:
                printf("[KEY = Gb Major]\n\n");
                break;
             case -7:
                 printf("[KEY = Cb Major]\n\n");
                 break;

            }
        }
        else if (keySigType == 1){
           switch (keySigKey)
            {
            case 7:
                printf("[KEY = A# Minor]\n");
                break;
            case 6:
                printf("[KEY = D# Minor]\n\n");
                break;
            case 5:
                printf("[KEY = G# Minor]\n\n");
                break;
            case 4:
                printf("[KEY = C# Minor]\n\n");
                break;
             case 3:
                printf("[KEY = F# Minor]\n\n");
                break;
            case 2:
                printf("[KEY = B Minor]\n\n]");
                break;
            case 1:
                printf("[KEY = E Minor]\n\n");
                break;
            case 0:
                printf("[KEY = A Minor]\n\n");
                break;
            case -1:
                printf("[KEY = D Minor]\n\n");
                break;
             case -2:
                printf("[KEY = G Minor]\n\n");
                break;
            case -3:
                printf("[KEY = C Minor]\n\n");
                break;
            case -4:
                printf("[KEY = F Minor]\n\n");
                break;
            case -5:
                printf("[KEY = Bb Minor]\n\n");
                break;
            case -6:
                printf("[KEY = Eb Minor]\n\n");
                break;
            case -7:
                printf("[KEY = Ab Minor]\n\n");
                break;
        }
             case MetaSequencerSpecific:
                 printf("meta sequencer specific... skipping message\n\n");
                fseek(fPointer, nLength, SEEK_CUR);
                break;

     }
    }

}

}

else {
    printf("unrecognized status byte!\n\n");
}



};



};
    fclose("open.mid");


    return 0;
}

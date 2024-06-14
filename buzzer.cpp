#include <Arduino.h>
#include "common.h"
#include "songs.h"
#include "light.h"

int songCount = 7; // 歌曲数量

void playSong(int note[],int duration[],int legth,float timeCoefficient){
  for (int thisNote = 0; thisNote < legth; thisNote++) {
    int noteDuration = 1000 / duration[thisNote];
    tone(BUZZER, note[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * timeCoefficient;
    delay(pauseBetweenNotes);
    noTone(BUZZER);
  }
}

void playSong(bool bell){
  int songNum;
  if(bell){
    songNum = clockBellNum;
  }else{
    songNum = tmpClockBellNum;
  }
  switch(songNum){
    case 0:
      playSong(SuperMario_note,SuperMario_duration,sizeof(SuperMario_note)/sizeof(int),2.1);
      break;
    case 1:
      playSong(AlwaysWithMe_note,AlwaysWithMe_duration,sizeof(AlwaysWithMe_note)/sizeof(int),3);
      break;
    case 2:
      playSong(DreamWedding_note,DreamWedding_duration,sizeof(DreamWedding_note)/sizeof(int),2.7);
      break;
    case 3:
      playSong(CastleInTheSky_note,CastleInTheSky_duration,sizeof(CastleInTheSky_note)/sizeof(int),2.8);
      break;
    case 4:
      playSong(Canon_note,Canon_duration,sizeof(Canon_note)/sizeof(int),1.8);
      break;
    case 5:
      playSong(HappyBirthday_note, HappyBirthday_duration,sizeof(HappyBirthday_note)/sizeof(int),1.2);
      break;
    case 6:
      playSong(HB_note,HB_duration,sizeof(HB_note)/sizeof(int), 1.5);
      break;
    default:
      break;
  }
}

// 随机挑选一首生日歌曲播放
void playBirthdaySong(){
  randomSeed(analogRead(RANDOM_SEED_PIN));
  int songNumber = random(0, 2);
  if(songNumber == 0){
    playSong(HappyBirthday_note, HappyBirthday_duration,sizeof(HappyBirthday_note)/sizeof(int),1.2);
  }else if(songNumber == 1){
    playSong(HB_note,HB_duration,sizeof(HB_note)/sizeof(int), 1.5);
  }
}
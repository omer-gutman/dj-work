#include "AudioTrack.h"
#include <iostream>
#include <cstring>
#include <random>

AudioTrack::AudioTrack(const std::string& title, const std::vector<std::string>& artists, 
                      int duration, int bpm, size_t waveform_samples)
    : title(title), artists(artists), duration_seconds(duration), bpm(bpm), 
      waveform_size(waveform_samples) {

    // Allocate memory for waveform analysis
    waveform_data = new double[waveform_size];

    // Generate some dummy waveform data for testing
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-1.0, 1.0);

    for (size_t i = 0; i < waveform_size; ++i) {
        waveform_data[i] = dis(gen);
    }
    #ifdef DEBUG
    std::cout << "AudioTrack created: " << title << " by " << std::endl;
    for (const auto& artist : artists) {
        std::cout << artist << " ";
    }
    std::cout << std::endl;
    #endif
}

// ========== TODO: STUDENTS IMPLEMENT RULE OF 5 ==========

AudioTrack::~AudioTrack() {
    // TODO: Implement the destructor
    #ifdef DEBUG
    std::cout << "AudioTrack destructor called for: " << title << std::endl;
    #endif
    delete[] this->waveform_data;
}

AudioTrack::AudioTrack(const AudioTrack& other)
  : title(other.title), 
    artists(other.artists), 
    duration_seconds(other.duration_seconds), 
    bpm(other.bpm), 
    waveform_size(other.waveform_size)
{
    // TODO: Implement the copy constructor
    #ifdef DEBUG
    std::cout << "AudioTrack copy constructor called for: " << other.title << std::endl;
    #endif
    //הקצאה חדשה של מקום נוסף בערימה להעתק:
    this->waveform_data = new double[this->waveform_size];
    /*פרמטרים של פקודת קופי:
    1. מצביע לנקודה ההתחלתית של מידע המקור - המידע שמעתיקים למיקום החדש.
    2. מצביע לנקודה אחת אחרי הנקודה האחרונה שממנה מעתיקים - משיגים את זה באמצעות כתובת התחלתית + (גודל מבנה הנתונים*גודל הטיפוס)
    3. מצביע לנקודת ההתחלה של האובייקט שאליו מעתיקים
    */
    std::copy(other.waveform_data, 
      other.waveform_data+other.waveform_size, 
      this->waveform_data);
}

AudioTrack& AudioTrack::operator=(const AudioTrack& other) {
    // TODO: Implement the copy assignment operator
    #ifdef DEBUG
    std::cout << "AudioTrack copy assignment called for: " << other.title << std::endl;
    #endif
    
    //1. נבדוק אם אנחנו לא מנסים להקצות לעצמנו (ת'יס = אות'ר)
    if(this == &other)
      return *this;
    //2. נמחק את המידע במיקום המקורי. (זה משחרר את המיקום לכן צריך להקצות מיקום חדש לתהליך).
    delete[] this->waveform_data;
    //3. העתק רדוד:
    this->title = other.title; //std::string
    this->artists = other.artists; //std::vector
    this->duration_seconds = other.duration_seconds; //int
    this->bpm = other.bpm; //int
    this->waveform_size = other.waveform_size; //int
    //4. נקצה מיקום חדש בערימה.
    this->waveform_data = new double[this->waveform_size];
    //5. נבצע העתק עמוק כמו במתודה הקודמת.
    std::copy(other.waveform_data, 
      other.waveform_data+other.waveform_size, 
      this->waveform_data);
    return *this;
}

AudioTrack::AudioTrack(AudioTrack&& other) noexcept 
    //נתחיל בהזזת הטיפוסים הפשוטים (מחרוזת ו-וקטור נעביר עם std::move).
    :   title(std::move(other.title)),
        artists(std::move(other.artists)),
        duration_seconds(other.duration_seconds), 
        bpm(other.bpm), 
        waveform_size(other.waveform_size),
        //"נגנוב" את המצביע
        waveform_data(other.waveform_data)
    {
    // TODO: Implement the move constructor
    #ifdef DEBUG
    std::cout << "AudioTrack move constructor called for: " << other.title << std::endl;
    #endif
    other.waveform_data = nullptr;
}

AudioTrack& AudioTrack::operator=(AudioTrack&& other) noexcept {
    // TODO: Implement the move assignment operator

    #ifdef DEBUG
    std::cout << "AudioTrack move assignment called for: " << other.title << std::endl;
    #endif
    //נתחיל בבדיקה שאנחנו לא "מזיזים את עצמנו"
    if(this == &other)
      return *this;
    //נמחק זיכרון ישן שלא יישאר ללא מצביע
    delete[] this->waveform_data;
    //נזיז את המידע בין האובייקטים בדומה למתודה הקודמת
    this->title = std::move(other.title); //std::string
    this->artists = std::move(other.artists); //std::vector
    this->duration_seconds = other.duration_seconds; //int
    this->bpm = other.bpm; //int
    this->waveform_size = other.waveform_size; //int
    this->waveform_data = other.waveform_data; //pointer
    //נדאג שהמצביע הקודם לא יצביע על המידע שאליו מצביע המצביע שלנו, כדי שהמידע לא יימחק בטעות ונימנע ממחיקה כפולה
    other.waveform_data = nullptr;
    return *this;
}

void AudioTrack::get_waveform_copy(double* buffer, size_t buffer_size) const {
    if (buffer && waveform_data && buffer_size <= waveform_size) {
        std::memcpy(buffer, waveform_data, buffer_size * sizeof(double));
    }

}


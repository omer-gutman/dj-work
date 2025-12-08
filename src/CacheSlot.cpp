#include "CacheSlot.h"

CacheSlot::CacheSlot() : 
    track(nullptr), 
    last_access_time(0), 
    occupied(false){
}
//Copy Constructor
CacheSlot::CacheSlot(const CacheSlot& other) 
    : track(nullptr), last_access_time(other.last_access_time), occupied(other.occupied) {
    
    if (other.occupied && other.track) {
        //מממשים את מודל הבעלות באמצעות שימוש ב-clone
        track = other.track->clone();
    }
}

//Copy Assignment Operator
CacheSlot& CacheSlot::operator=(const CacheSlot& other) {
    if (this == &other) return *this;
    
    // ניקוי הנוכחי
    track.reset();
    
    // העתקת נתונים פשוטים
    last_access_time = other.last_access_time;
    occupied = other.occupied;
    
    // העתקה עמוקה של הטראק
    if (other.occupied && other.track) {
        track = other.track->clone();
    }
    
    return *this;
}

void CacheSlot::store(PointerWrapper<AudioTrack> track_ptr, uint64_t access_time) {
    track = std::move(track_ptr);
    last_access_time = access_time;
    occupied = true;
}

AudioTrack* CacheSlot::access(uint64_t access_time) {
    if (!occupied) {
        return nullptr;
    }
    
    last_access_time = access_time;
    return track.get();
}

void CacheSlot::clear() {
    track.reset(nullptr);
    occupied = false;
    last_access_time = 0;
}
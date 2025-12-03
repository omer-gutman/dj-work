#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
    std::string title = track.get_title();

    // 1. בודקים אם יש לנו קומבינה והטראק כבר בפנים
    if (cache.contains(title)) {
        cache.get(title); // מרעננים אותו שיהיה בראש הרשימה
        return 1; // HIT - שיחוק!
    }

    // 2. MISS - צריך לעבוד. משכפלים את הטראק, שלא נהרוס את המקורי בספרייה
    PointerWrapper<AudioTrack> clone = track.clone();
    
    if (!clone) {
        std::cout << "[ERROR] Track: \"" << title << "\" failed to clone\n";
        return 0; // נפלנו בשיבוט, איזה פח
    }

    // 3. עושים לו טיפול 10,000 לפני שנכנס לקאש
    AudioTrack* rawClone = clone.get(); // מחלצים את הפוינטר רגע
    if (rawClone) {
        rawClone->load(); // טוענים
        rawClone->analyze_beatgrid(); // בודקים את הביטים
    }

    // 4. דוחפים לקאש. שימו לב ל-move, מעבירים בעלות כמו גברים
    bool evictionOccurred = cache.put(std::move(clone));

    if (evictionOccurred) {
        return -1; // MISS + EVICTION (העפנו מישהו החוצה)
    }
    return 0; // MISS רגיל (נכנס חלק)
}

void DJControllerService::set_cache_size(size_t new_size) {
    cache.set_capacity(new_size);
}
//implemented
void DJControllerService::displayCacheStatus() const {
    std::cout << "\n=== Cache Status ===\n";
    cache.displayStatus();
    std::cout << "====================\n";
}

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
    // פשוט מבקשים יפה מהקאש
    return cache.get(track_title);
}

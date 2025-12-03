#include "MixingEngineService.h"
#include <iostream>
#include <memory>
#include <cmath> // בשביל הערך המוחלט, שלא נצא מינוס


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : active_deck(0), auto_sync(false), bpm_tolerance(0)
{
    decks[0] = nullptr;
    decks[1] = nullptr;
    std::cout << "[MixingEngineService] Initialized with 2 empty decks. יאללה בלאגן\n";
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    std::cout << "[MixingEngineService] Cleaning up decks. סוגרים את הבאסטה\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i]) {
            delete decks[i]; // מנקים שאריות
            decks[i] = nullptr;
        }
    }
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    std::cout << "\n=== Loading Track to Deck ===\n";

    // 1. משכפלים! אסור לגעת במקור מהקאש, זה קדוש
    PointerWrapper<AudioTrack> clone = track.clone();
    if (!clone) {
        std::cout << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone\n";
        return -1;
    }

    // 2. מחפשים את הדק הפנוי (זה שלא מנגן כרגע)
    // אם active הוא 0, הפנוי הוא 1. אם active הוא 1, הפנוי הוא 0. מתמטיקה של כיתה א'
    size_t target_deck = 1 - active_deck;
    std::cout << "[Deck Switch] Target deck: " << target_deck << "\n";

    // 3. אם יש שם מישהו ישן, מעיפים אותו לפח
    if (decks[target_deck]) {
        delete decks[target_deck];
        decks[target_deck] = nullptr;
    }

    // 4. מכינים את השכפול לשידור
    AudioTrack* rawClone = clone.get();
    if (rawClone) {
        rawClone->load();
        rawClone->analyze_beatgrid();
    }

    // 5. בדיקת סנכרון (Auto Sync)
    // אם הדק הפעיל מנגן, ויש לנו אישור לסנכרן אוטומטית...
    if (decks[active_deck] && auto_sync) {
        // אם הם לא מסתדרים בקצב (BPM), עושים גישור
        if (!can_mix_tracks(clone)) {
             sync_bpm(clone); // יאללה תיישר קו
        }
    }

    // 6. מעבירים בעלות לדק (משחררים מהעטיפה)
    decks[target_deck] = clone.release();
    std::cout << "[Load Complete] '" << decks[target_deck]->get_title() << "' is now loaded on deck " << target_deck << "\n";

    // 7. Instant Transition - המעבר החד!
    // מעיפים את הדק הקודם מיד אחרי שהחדש מוכן
    if (decks[active_deck]) {
        std::cout << "[Unload] Unloading previous deck " << active_deck << " (" << decks[active_deck]->get_title() << ")\n";
        delete decks[active_deck]; // ביי ביי
        decks[active_deck] = nullptr;
    }

    // 8. מחליפים את הדק הפעיל - עכשיו תורך לזרוח
    active_deck = target_deck;
    std::cout << "[Active Deck] Switched to deck " << target_deck << "\n";

    return target_deck;
}

/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    // אם אין עם מי לדבר, או שהטראק החדש דפוק, אין מיקס
    if (!decks[active_deck] || !track) {
        return false;
    }

    float currentBpm = decks[active_deck]->get_bpm();
    float newBpm = track->get_bpm();
    // בודקים את ההפרש בערך מוחלט
    float diff = std::abs(currentBpm - newBpm);

    // אם ההפרש קטן מהסובלנות, סבבה. אחרת, לא עובר סלקציה
    return diff <= bpm_tolerance;
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    if (!decks[active_deck] || !track) {
        return;
    }

    float originalBpm = track->get_bpm();
    float activeBpm = decks[active_deck]->get_bpm();
    
    // עושים ממוצע כמו בשוק, חצי חצי
    float avgBpm = (originalBpm + activeBpm) / 2.0f;

    track->set_bpm(avgBpm); // מעדכנים את הטראק החדש

    std::cout << "[Sync BPM] Syncing BPM from " << originalBpm << " to " << avgBpm << " (פשרה הוגנת)\n";
}
#include "LRUCache.h"
#include <iostream>
#include <limits>

namespace {
    // שלא יהיה פדיחות עם מספרים קסומים, נגדיר קבוע
    constexpr size_t INVALID_INDEX = -1; 
}

LRUCache::LRUCache(size_t capacity)
    : slots(capacity), max_size(capacity), access_counter(0) {}

bool LRUCache::contains(const std::string& track_id) const {
    return findSlot(track_id) != max_size;
}

AudioTrack* LRUCache::get(const std::string& track_id) {
    size_t idx = findSlot(track_id);
    if (idx == max_size) return nullptr; // וואלה לא מצאנו, דבר ללמפה

    // מצאנו את הבחור! תן לו בראש עם זמן גישה חדש, יענו הוא עכשיו המלך של הרחבה (MRU)
    return slots[idx].access(++access_counter);
}

/**
 * TODO: Implement the put() method for LRUCache
 */
bool LRUCache::put(PointerWrapper<AudioTrack> track) {
    if (!track) return false; // הביאו לי אוויר, מה אני אעשה עם זה?

    std::string title = track->get_title();
    size_t existingIdx = findSlot(title);

    // תרחיש 1: הטראק כבר בפנים, יושב רגל על רגל
    if (existingIdx != max_size) {
        // רק מעדכנים לו את הפז"ם שיהיה טרי, ולא מכניסים כלום
        slots[existingIdx].access(++access_counter);
        // ה-PointerWrapper החדש שהגיע מתפוגג פה, לא צריך אותו
        return false; 
    }

    // תרחיש 2: אורח חדש הגיע לשכונה
    bool evicted = false;
    size_t insertIdx = findEmptySlot();

    // המועדון מלא פיצוץ? אין מקום לסיכה?
    if (insertIdx == max_size) {
        // יאללה להעיף את מי שיבש הכי הרבה זמן (LRU)
        if (evictLRU()) {
            evicted = true; // סימנו וי, מישהו הלך הביתה
        }
        insertIdx = findEmptySlot(); // עכשיו בטוח יש מקום
    }

    // יאללה כנס, שים אותו במשבצת
    if (insertIdx != max_size) {
        slots[insertIdx].store(std::move(track), ++access_counter);
    }

    return evicted; // מחזירים אם עשינו בלאגן והעפנו מישהו
}

bool LRUCache::evictLRU() {
    size_t lruIdx = findLRUSlot();
    // אם אין את מי להעיף, או שהמקום ריק, עזוב אותי באמאשך
    if (lruIdx == max_size || !slots[lruIdx].isOccupied()) {
        return false;
    }
    
    // יאללה קישטא, מפנים את הסלוט
    slots[lruIdx].clear();
    return true;
}

size_t LRUCache::size() const {
    size_t count = 0;
    for (const auto& slot : slots) if (slot.isOccupied()) ++count;
    return count;
}

void LRUCache::clear() {
    for (auto& slot : slots) {
        slot.clear();
    }
}

void LRUCache::displayStatus() const {
    std::cout << "[LRUCache] Status: " << size() << "/" << max_size << " slots used\n";
    for (size_t i = 0; i < max_size; ++i) {
        if(slots[i].isOccupied()){
            std::cout << "  Slot " << i << ": " << slots[i].getTrack()->get_title()
                      << " (last access: " << slots[i].getLastAccessTime() << ")\n";
        } else {
            std::cout << "  Slot " << i << ": [EMPTY]\n";
        }
    }
}

size_t LRUCache::findSlot(const std::string& track_id) const {
    for (size_t i = 0; i < max_size; ++i) {
        // מחפשים מחט בערימת שחת, אבל לפי שם השיר
        if (slots[i].isOccupied() && slots[i].getTrack()->get_title() == track_id) return i;
    }
    return max_size; // לא נמצא, באסה
}

/**
 * TODO: Implement the findLRUSlot() method for LRUCache
 */
size_t LRUCache::findLRUSlot() const {
    size_t lruIdx = max_size;
    uint64_t minAccess = std::numeric_limits<uint64_t>::max(); // מספר ענק, אינסוף יענו

    // עוברים אחד אחד ובודקים מי הזקן של השבט (זמן גישה הכי נמוך)
    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied()) {
            if (slots[i].getLastAccessTime() < minAccess) {
                minAccess = slots[i].getLastAccessTime();
                lruIdx = i;
            }
        }
    }
    return lruIdx;
}

size_t LRUCache::findEmptySlot() const {
    // מחפשים חניה פנויה בתל אביב
    for (size_t i = 0; i < max_size; ++i) {
        if (!slots[i].isOccupied()) return i;
    }
    return max_size; // אין חניה, סע לחניון
}

void LRUCache::set_capacity(size_t capacity){
    if (max_size == capacity)
        return;
    max_size = capacity;
    slots.resize(capacity);
}
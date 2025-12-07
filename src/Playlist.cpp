#include "Playlist.h"
#include "AudioTrack.h"
#include <iostream>
#include <algorithm>
Playlist::Playlist(const std::string& name) 
    : head(nullptr), playlist_name(name), track_count(0) {
    std::cout << "Created playlist: " << name << std::endl;
}
// TODO: Fix memory leaks!
// Students must fix this in Phase 1

Playlist::~Playlist() {
    PlaylistNode* current = head;
    //מוחק גם את הנודים
    while (current != nullptr) {
        PlaylistNode* next_node = current->next;
        delete current; 
        current = next_node;
    }
    #ifdef DEBUG
    std::cout << "Destroying playlist: " << playlist_name << std::endl;
    #endif
}
//מימשנו את הדיסטרקטור אז צריך לממש את חוק ה-3
Playlist::Playlist(const Playlist& other) {
    //העתקת משתנים פשוטים
    this->playlist_name = other.playlist_name; //std::string
    this->track_count = other.track_count; //int
    this->head = nullptr;
    //current - מצביע שיעקוב אחרי other
    PlaylistNode* current = other.head;
    //tail - מצביע שישמור על זנב הרשימה החדשה ויחבר כל פעם לראש ויתקדם
    //הכרחי כדי לשמור על ראש הרשימה החדשה ולא להפוך אותה.
    PlaylistNode* tail = nullptr;

    while(current) {
        AudioTrack* temp = current->track->clone().release();
        PlaylistNode* node = new PlaylistNode(temp);
        if(this->head == nullptr) {this->head = node; }
        else {tail->next = node; }
        current = current->next;
        tail = node;
    }
}
//מימשנו את הדיסטרקטור אז צריך לממש את חוק ה-3
Playlist &Playlist::operator=(const Playlist& other)
{
    if(this == &other) return *this;
    Playlist temp(other);
    std::swap(*this, temp);
    return *this;
}

void Playlist::add_track(AudioTrack* track) {
    if (!track) {
        std::cout << "[Error] Cannot add null track to playlist" << std::endl;
        return;
    }

    // Create new node - this allocates memory!
    PlaylistNode* new_node = new PlaylistNode(track);

    // Add to front of list
    new_node->next = head;
    head = new_node;
    track_count++;

    std::cout << "Added '" << track->get_title() << "' to playlist '" 
              << playlist_name << "'" << std::endl;
}



void Playlist::remove_track(const std::string& title) {
    PlaylistNode* current = head;
    PlaylistNode* prev = nullptr;

    // Find the track to remove
    while (current && current->track->get_title() != title) {
        prev = current;
        current = current->next;
    }

    
    if (current) {
        if (prev) {
            prev->next = current->next;
        } else {
            head = current->next;
        }

        track_count--;
        std::cout << "Removed '" << title << "' from playlist" << std::endl;

        delete current; // היה חסר
    } else {
        std::cout << "Track '" << title << "' not found in playlist" << std::endl;
    }
}

void Playlist::display() const {
    std::cout << "\n=== Playlist: " << playlist_name << " ===" << std::endl;
    std::cout << "Track count: " << track_count << std::endl;

    PlaylistNode* current = head;
    int index = 1;

    while (current) {
        std::vector<std::string> artists = current->track->get_artists();
        std::string artist_list;

        std::for_each(artists.begin(), artists.end(), [&](const std::string& artist) {
            if (!artist_list.empty()) {
                artist_list += ", ";
            }
            artist_list += artist;
        });

        AudioTrack* track = current->track.get(); //added .get()
        std::cout << index << ". " << track->get_title() 
                  << " by " << artist_list
                  << " (" << track->get_duration() << "s, " 
                  << track->get_bpm() << " BPM)" << std::endl;
        current = current->next;
        index++;
    }

    if (track_count == 0) {
        std::cout << "(Empty playlist)" << std::endl;
    }
    std::cout << "========================\n" << std::endl;
}

AudioTrack* Playlist::find_track(const std::string& title) const {
    PlaylistNode* current = head;

    while (current) {
        if (current->track->get_title() == title) {
            return current->track.get(); //הוספנו get()
        }
        current = current->next;
    }

    return nullptr;
}

int Playlist::get_total_duration() const {
    int total = 0;
    PlaylistNode* current = head;

    while (current) {
        total += current->track->get_duration();
        current = current->next;
    }

    return total;
}

std::vector<AudioTrack*> Playlist::getTracks() const {
    std::vector<AudioTrack*> tracks;
    PlaylistNode* current = head;
    while (current) {
        if (current->track)
            tracks.push_back(current->track.get()); //הוספנו get()
        current = current->next;
    }
    return tracks;
}
#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist) {}

//בגלל שוקטור הספריה מכיל מצביעים רגילים ולא חכמים, הבעלות עליהם נמצאת במחלקה ולכן צריך למחוק אותם בסוף השימוש.
DJLibraryService::~DJLibraryService() {
    for(const AudioTrack* track : this->library) {delete track;}
}
//בגלל שמימשנו את הdestructor, לפי חוק ה-3 נממש גם את 2 בנאי ההעתקה:

DJLibraryService::DJLibraryService(const DJLibraryService& other) : playlist(other.playlist) {
    for(const AudioTrack* track : other.library) {
        this->library.push_back(track->clone().release());
    }
}

DJLibraryService& DJLibraryService:: operator=(const DJLibraryService& other){
    if (this == &other) return *this;
    DJLibraryService temp(other);
    swap(temp);
    return *this;
}

//מתודה משלימה/ עוזרת - החלפה של כל אחד מהאובייקטים של המחלקה.
void DJLibraryService::swap(DJLibraryService& other) {
    std::swap(this->playlist, other.playlist);
    std::swap(this->library, other.library);
}

/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 * ממומש - המתודה בודקת כל טראק אם הוא MP3 או WAV ומכניסה לספריה. הפרמטרים מאותחלים לאחר בדיקה.
 * המתודה גם מסירה את המצביע החכם - תומך את מודל הבעלות של המחלקה.
 * אנו מממשים את האחריות בDestructor.
 */
void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
    //Todo: Implement buildLibrary method
    for(const SessionConfig::TrackInfo& info : library_tracks) {
        if(info.type == "MP3") {
            PointerWrapper<AudioTrack> track(new MP3Track(info.title, info.artists, 
                info.duration_seconds, info.bpm, info.extra_param1, (bool)info.extra_param2));
            this->library.push_back(track.release());
        }
        else { //if(info.type == "WAV")
            PointerWrapper<AudioTrack> track(new WAVTrack(info.title, info.artists, 
                info.duration_seconds, info.bpm, info.extra_param1, info.extra_param2));
            this->library.push_back(track.release());
        }
    }
    std::cout << "TODO: Implement DJLibraryService::buildLibrary method\n"<< library_tracks.size() << " tracks to be loaded into library.\n";
}



/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: " 
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    return playlist;
}

/**
 *המתודה עוברת על השירים בספריה ומחזירה את השיר המתאים לפי שם.
 *המתודה מחזירה מצביע const - תומך במודל הבעלות של המחלקה.
 *לא השתמשנו במתודה find_track של Playlist כי ייתכן וקיימים שירים בספריה שאינם בפלייליסט ואנחנו עשויים לפספס אותם.
 */
const AudioTrack* DJLibraryService::findTrack(const std::string& track_title) const {
    for(const AudioTrack* track : this->library) {
        if ((*track).get_title() == track_title) return track;
    }
    std::cout << "Track Doesn't Exist In Library" << std::endl;
    return nullptr;
}

/*
*במתודה זו אנו מאתחלים את הפלייליסט שלנו (למקרה שאינו ריק)
*עבור כל מספר בtrack_indices מאתחלים את השיר המתאים מהספריה
*ומוסיפים לפלייליסט.
*בתום המתודה, playlist מכיל את כל השירים הרלוונטיים לאחר אתחולם.
*/
void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name, 
                                               const std::vector<int>& track_indices) {
    Playlist empty(playlist_name);
    this->playlist = empty;
    for(int i : track_indices) {
        if(i > 0 && i <= this->library.size()) {
            AudioTrack* temp = this->library[i-1]->clone().release();
            temp->load();
            temp->analyze_beatgrid();
            this->playlist.add_track(temp);
        }
    }
}
/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 * מומש - המתודה מחזירה וקטור שמות כל השירים שקיימים בספריה.
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    std::vector<AudioTrack*> temp = this->playlist.getTracks();
    std::vector<std::string> titles;
    for(const AudioTrack* track : temp) {
        titles.push_back(track->get_title());
    }
    return titles;
}
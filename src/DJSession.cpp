
#include "DJSession.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>

// ========== CONSTRUCTORS & RULE OF 5 ==========


DJSession::DJSession(const std::string& name, bool play_all)
    : session_name(name), play_all(play_all) {
    std::cout << "DJ Session System initialized: " << session_name << std::endl;
}

//Destructor
DJSession::~DJSession() {
    std::cout << "Shutting down DJ Session System: " << session_name << std::endl;
}
//Copy Constructor
DJSession::DJSession(const DJSession& other) 
    : session_name(other.session_name), 
      library_service(other.library_service), // קורא ל-Copy Constructor של DJLibraryService
      controller_service(other.controller_service), // קורא ל-Copy Constructor של DJControllerService
      mixing_service(other.mixing_service), // קורא ל-Copy Constructor של MixingEngineService
      session_config(other.session_config), // Copy Constructor סטנדרטי
      track_titles(other.track_titles),
      play_all(other.play_all),
      stats(other.stats)
{
    // הגוף נשאר ריק מכיוון שכל ההעתקה מתבצעת ברשימת האתחול (Initializer List)
    std::cout << "DJ Session copy constructor called for: " << session_name << std::endl;
}

//Copy Assignment Operator
DJSession& DJSession::operator=(const DJSession& other) {
    if (this == &other) {return *this;}
    DJSession temp(other);
    this->swap(temp);
    return *this;
}

// בתוך DJSession.cpp (או כ-member function)

// מתודת עזר פנימית להחלפת כל המשאבים
void DJSession::swap(DJSession& other) noexcept {
    // החלפת המשאבים המורכבים (הם כבר Copy/Move Safe)
    std::swap(session_name, other.session_name);
    std::swap(library_service, other.library_service); 
    std::swap(controller_service, other.controller_service);
    std::swap(mixing_service, other.mixing_service);
    
    // החלפת חברי הנתונים הפשוטים והסטטיסטיקות
    std::swap(config_manager, other.config_manager);
    std::swap(session_config, other.session_config);
    std::swap(track_titles, other.track_titles);
    std::swap(play_all, other.play_all);
    std::swap(stats, other.stats);
}

// ========== CORE FUNCTIONALITY ==========
bool DJSession::load_playlist(const std::string& playlist_name)  {
    std::cout << "[System] Loading playlist: " << playlist_name << "\n";
    
    // Find the playlist in the session config
    auto it = session_config.playlists.find(playlist_name);
    if (it == session_config.playlists.end()) {
        std::cerr << "[ERROR] Playlist '" << playlist_name << "' not found in configuration.\n";
        return false;
    }
    
    // Load playlist from track indices
    library_service.loadPlaylistFromIndices(playlist_name, it->second);
    
    if (library_service.getPlaylist().is_empty()) {
        return false;
    }
    
    track_titles = library_service.getTrackTitles();
    return true;
}

/**
 * TODO: Implement load_track_to_controller method
 * 
 * REQUIREMENTS:
 * 1. Track Retrieval
 *    - Find track in library using track name
 *    - Handle case when track is not found
 *    - Update error stats if track not found
 * 
 * 2. Controller Loading
 *    - Delegate loading to controller_service
 *    - Pass track by reference to controller
 * 
 * 3. Return Values
 *    1: Cache HIT
 *    0: Cache MISS (or error)
 *   -1: Cache MISS with eviction
 * 
 * @param track_name: Name of track to load
 * @return: Cache operation result code

 */
int DJSession::load_track_to_controller(const std::string& track_name) {
    //1. Track Retrieval: 
    const AudioTrack* track(library_service.findTrack(track_name)); 
    if (!track) {
        std::cout << "[ERROR] track name is not in library." << std::endl;
        stats.errors++;
        return 0;
    }
    //2. Controller Loading: 
    //const_cast - telling the program to ignore the const. Need to pass a reference, not a const ptr.
    int result = controller_service.loadTrackToCache(const_cast<AudioTrack&>(*track));
    switch (result) {
        case 1: 
            stats.cache_hits++;
            break;
        case 0: 
            stats.cache_misses++;
            break;
        case -1:
            stats.cache_evictions++;
            stats.cache_misses++;
            break;
        default:
            std::cout << "[ERROR] impossible return value from loadTrackToCache. Stats unchanged." << std::endl;
    }
    return result;
}

/**
 * TODO: Implement load_track_to_mixer_deck method
 * 
 * @param track_title: Title of track to load to mixer
 * @return: Whether track was successfully loaded to a deck
 */
bool DJSession::load_track_to_mixer_deck(const std::string& track_title) {
    std::cout << "[System] Delegating track transfer to MixingEngineService for: " << track_title << std::endl;
    AudioTrack* track = controller_service.getTrackFromCache(track_title);
    if(!track) {
        std::cout << "[ERROR] MISS: load_to_mixer_deck failed: track doesn't exist." << std::endl;
        stats.errors++;
        return false;
    }
    int deck = mixing_service.loadTrackToDeck((*track));
    switch (deck) {
        case 0:
            std::cout << "Track Loaded to Deck A" << std::endl;
            stats.deck_loads_a++;
            stats.transitions++;
            break;
        case 1: 
            std::cout << "Track Loaded to Deck B" << std::endl;
            stats.deck_loads_b++;
            stats.transitions++;
            break;
        case -1:
            std::cout << "Track Loading to Deck Failed." << std::endl;
            stats.errors++;
            break;
        default:
            break;
    }
    return deck == 1 || deck == 0; 
}

/**
 * @brief Main simulation loop that orchestrates the DJ performance session.
 * @note Updates session statistics (stats) throughout processing
 * @note Calls print_session_summary() to display results after playlist completion
 */
void DJSession::simulate_dj_performance() {
    std::cout << "=== DJ Controller System ===" << std::endl;
    std::cout << "Starting interactive DJ session..." << std::endl;
    // 1. Load configuration
    if (!load_configuration()) {
        std::cerr << "[ERROR] Failed to load configuration. Aborting session." << std::endl;
        return;
    }
    
    // 2. Build track library from config
    library_service.buildLibrary(session_config.library_tracks);
    
    // 3. Get available playlists from config
    if (session_config.playlists.empty()) {
        std::cerr << "[ERROR] No playlists found in configuration. Aborting session." << std::endl;
        return;
    }
    std::cout << "\nStarting DJ performance simulation..." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Capacity: " << session_config.controller_cache_size << " slots (LRU policy)" << std::endl;
    std::cout << "\n--- Processing Tracks ---" << std::endl;

    std::cout << "TODO: Implement the DJ performance simulation workflow here." << std::endl;
    //איטרטור שרץ על המפה
    auto playlist_it = session_config.playlists.begin();
    std::string selected_playlist_name;
    
    while(true) {
        if (play_all) { //מצב אוטומטי - איטרציה על כל הלולאות בעזרת איטרטור על המפה
            if (playlist_it == session_config.playlists.end()) {
                break; // נגמרו הפלייליסטים - יציאה מהלולאה
            }
            // האיטרטור מחזיר pair<string, vector<int>>, ניקח רק את השם (first)
            selected_playlist_name = playlist_it->first; 
            playlist_it++; // מקדמים לאיטרציה הבאה
            
        } 
        else {
            // מצב אינטראקטיבי: מקבלים בחירה מהמשתמש
            selected_playlist_name = display_playlist_menu_from_config();
            if (selected_playlist_name.empty()) {
                break; // המשתמש בחר 0 (Cancel) - יציאה מהלולאה
            }
        }
        
        //continue אם הפלייליסט ריק או לא קיים
        if (!load_playlist(selected_playlist_name)) {
            continue; 
        }
        
        stats.transitions = 0; // מאפסים ספירת מעברים עבור כל פלייליסט 
        
        // עוברים על רשימת כותרות השירים שנטענה בתוך load_playlist
        for (const std::string& track_title : track_titles) {
            load_track_to_controller(track_title);
            load_track_to_mixer_deck(track_title);
            stats.tracks_processed++; // מעדכנים סה"כ שירים שעיבדנו
        }

        print_session_summary();
        //מאפסים סטטיסטיקות ספציפיות לפלייליסט הזה
        stats.cache_hits = 0;
        stats.cache_misses = 0;
        stats.cache_evictions = 0;
        stats.deck_loads_a = 0;
        stats.deck_loads_b = 0;
        stats.transitions = 0;
    }
}


/* 
 * Helper method to load session configuration from file
 * 
 * @return: true if configuration loaded successfully; false on error
 */
bool DJSession::load_configuration() {
    const std::string config_path = "bin/dj_config.txt";
    
    std::cout << "Loading configuration from: " << config_path << std::endl;
    
    if (!SessionFileParser::parse_config_file(config_path, session_config)) {
        std::cerr << "[ERROR] Failed to parse configuration file: " << config_path << std::endl;
        return false;
    }
    
    std::cout << "Configuration loaded successfully." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Size: " << session_config.controller_cache_size << " slots" << std::endl;
    mixing_service.set_auto_sync(session_config.auto_sync);
    mixing_service.set_bpm_tolerance(session_config.bpm_tolerance);
    //update cache size in LRUCache
    controller_service.set_cache_size(session_config.controller_cache_size);
    return true;
}

std::string DJSession::display_playlist_menu_from_config() {
    if (session_config.playlists.empty()) {
        return "";
    }
    
    std::cout << "\n=== Available Playlists ===" << std::endl;
    
    // Build sorted list of playlist names
    std::vector<std::string> playlist_names;
    for (const auto& pair : session_config.playlists) {
        playlist_names.push_back(pair.first);
    }
    std::sort(playlist_names.begin(), playlist_names.end());
    
    // Display numbered list
    for (size_t i = 0; i < playlist_names.size(); ++i) {
        std::cout << (i + 1) << ". " << playlist_names[i] << std::endl;
    }
    std::cout << "0. Cancel" << std::endl;
    
    // Prompt for user selection with validation
    int selection = -1;
    while (true) {
        std::cout << "\nSelect a playlist (1-" << playlist_names.size() << ", 0 to cancel): ";
        std::string input;
        
        if (!std::getline(std::cin, input)) {
            std::cout << "\n[ERROR] Input error. Cancelling session." << std::endl;
            return "";
        }
        
        std::stringstream ss(input);
        if (ss >> selection && ss.eof()) {
            if (selection == 0) {
                return "";
            } else if (selection >= 1 && selection <= static_cast<int>(playlist_names.size())) {
                std::string selected_name = playlist_names[selection - 1];
                std::cout << "Selected: " << selected_name << std::endl;
                return selected_name;
            }
        }
        
        std::cout << "Invalid selection. Please enter a number between 1 and " 
                  << playlist_names.size() << ", or 0 to cancel." << std::endl;
    }
}

void DJSession::print_session_summary() const {
    std::cout << "\n=== DJ Session Summary ===" << std::endl;
    std::cout << "Session: " << session_name << std::endl;
    std::cout << "Tracks processed: " << stats.tracks_processed << std::endl;
    std::cout << "Cache hits: " << stats.cache_hits << std::endl;
    std::cout << "Cache misses: " << stats.cache_misses << std::endl;
    std::cout << "Cache evictions: " << stats.cache_evictions << std::endl;
    std::cout << "Deck A loads: " << stats.deck_loads_a << std::endl;
    std::cout << "Deck B loads: " << stats.deck_loads_b << std::endl;
    std::cout << "Transitions: " << stats.transitions << std::endl;
    std::cout << "Errors: " << stats.errors << std::endl;
    std::cout << "=== Session Complete ===" << std::endl;
}
import json
import os
import time
import serial
import musicbrainzngs
from unidecode import unidecode

SETTINGS_FILE = "settings.json"

def load_settings():
    if not os.path.exists(SETTINGS_FILE):
        default_settings = {
            "serial_port": "/dev/ttyACM0",
            "baudrate": 9600,
            "useragent_app": "mds_labelgen",
            "useragent_version": "1.0",
            "useragent_contact": "example@example.com"
        }
        with open(SETTINGS_FILE, "w") as f:
            json.dump(default_settings, f, indent=2)
        print("Created settings.json with default values.")
        return default_settings
    else:
        with open(SETTINGS_FILE, "r") as f:
            return json.load(f)

def ascii_clean(s):
    return unidecode(s)

def fetch_albums(artist, album, settings):
    musicbrainzngs.set_useragent(
        settings["useragent_app"],
        settings["useragent_version"],
        settings["useragent_contact"]
    )

    result = musicbrainzngs.search_releases(artist=artist, release=album, limit=10)
    return result["release-list"]

def fetch_release_details(mbid):
    return musicbrainzngs.get_release_by_id(mbid, includes=["recordings", "artists"])

def generate_label_file(artist, album, tracklist):
    filename = "md_title.txt"
    with open(filename, "w", encoding="ascii", errors="ignore") as f:
        f.write(f"00{artist} - {album}\n")
        for i, track in enumerate(tracklist):
            num = f"{i+1:02}"
            f.write(f"{num}{track}\n")
    print(f"\nMiniDisc label file saved: {filename}")
    return filename

def send_over_serial(file_path, settings):
    port = settings["serial_port"]
    baudrate = settings.get("baudrate", 9600)

    try:
        with serial.Serial(port, baudrate, timeout=1) as ser, open(file_path, "r") as f:
            for i, line in enumerate(f):
                ser.write(line.strip().encode("ascii") + b"")
                print(f"Sent to serial ({i:02}): {line.strip()}")
                time.sleep(2)
    except Exception as e:
        print(f"Serial port error: {e}")

def main():
    settings = load_settings()

    while True:
        artist = input("Artist name: ").strip()
        album = input("Album title: ").strip()

        releases = fetch_albums(artist, album, settings)
        if not releases:
            print("No results found.")
            continue

        while True:
            print("\nResults:")
            for i, r in enumerate(releases):
                title = r.get("title", "unknown")
                artist_name = r["artist-credit"][0].get("name", "unknown")
                date = r.get("date", "N/A")
                print(f"{i+1}. {artist_name} – {title} ({date})")
            print("Press 'q' to quit.")

            choice = input("Select number: ").strip().lower()
            if choice == "q":
                break

            try:
                idx = int(choice) - 1
                release_id = releases[idx]["id"]
            except (IndexError, ValueError):
                print("Invalid selection.")
                continue

            details = fetch_release_details(release_id)["release"]
            media = details["medium-list"][0]
            tracks_raw = media["track-list"]

            print("\nTracklist:")
            for i, track in enumerate(tracks_raw):
                title = track["recording"]["title"]
                length_ms = int(track.get("length", 0))
                length_sec = length_ms // 1000
                minutes = length_sec // 60
                seconds = length_sec % 60
                print(f"{i+1:02}. {title} ({minutes}:{seconds:02})")

            ok = input("Is this the correct album? (y/n): ").strip().lower()
            if ok == "y":
                break

        if choice == "q":
            continue

        artist_credit = details.get("artist-credit", [])
        if artist_credit and isinstance(artist_credit, list) and "artist" in artist_credit[0] and "name" in artist_credit[0]["artist"]:
            artist_name = ascii_clean(artist_credit[0]["artist"]["name"])
        else:
            artist_name = ascii_clean(artist)

        album_title = ascii_clean(details.get("title", album))
        tracklist = [ascii_clean(t["recording"]["title"]) for t in tracks_raw]

        filepath = generate_label_file(artist_name, album_title, tracklist)
        send_over_serial(filepath, settings)

if __name__ == "__main__":
    main()

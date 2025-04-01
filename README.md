# mds-link
Sony MiniDisc S-Link adapter

This is a small arduino program to use the Sony S-Link protocol for disk and track name writing to a Sony MiniDisc player/recorder.

# mds_labelgen

This Python script is a **MiniDisc label generator** that uses the MusicBrainz database to fetch the tracklist of a selected album by artist name. 
The track titles are converted to ASCII and written to a text file (`md_title.txt`), which is then transmitted over a serial port to a MiniDisc labeling device.

## 🎯 Features

- Search for an album by artist and title using the MusicBrainz API
- Browse and select from multiple search results
- Preview full tracklist before confirmation
- Automatically convert all characters to ASCII
- Generate a properly formatted MiniDisc label file
- Transmit the label file line-by-line via serial port

## ⚙️ Settings

The first run will create a `settings.json` file if it doesn't exist:

```json
{
  "serial_port": "/dev/ttyACM0",
  "baudrate": 9600,
  "useragent_app": "mds_labelgen",
  "useragent_version": "1.0",
  "useragent_contact": "example@example.com"
}
```

You can customize these settings to match your serial port configuration or provide your own MusicBrainz user-agent identity.

## 🧰 Requirements

Install dependencies using pip:

```bash
pip install musicbrainzngs pyserial unidecode
```

## ▶️ Usage

Start the program from the terminal:

```bash
python3 mds_labelgen.py
```

Follow the prompts:

1. Enter the artist's name
2. Enter the album title
3. Browse matching releases and select one by number
4. Preview the tracklist
5. Confirm with `y` if correct, or `n` to return to the list
6. A label file (`md_title.txt`) will be created
7. The file will be transmitted over the specified serial port

## 📤 Serial Transmission

Each line of the `md_title.txt` file is sent through the serial port with a short delay. You can adjust the `baudrate` or port in `settings.json`.

## 📝 Notes

- Only ASCII characters are written to the output file and serial stream to ensure MiniDisc compatibility.
- If no album is found, you will be prompted to enter a new artist/title.
- To exit the selection menu, press `q`.

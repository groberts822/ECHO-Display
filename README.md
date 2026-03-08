# ECHO-Display
ECHO, or Embedded Controller &amp; Hidden Operations. Music display by day, game console by night.
To begin, I started working on my schematic. It was quite simple, I chose to 2 1x7 headers instead of going through the pain to spend forever searching through the internet for the right symbol and footprint of the esp32. 
I prefer to use net labels, especially with schematics that are this simple, so here's my schematic below:

![image](https://stasis.hackclub-assets.com/images/1772986942265-sx02dq.png)

In addition, my PCB was quite simple, however it took some measurements, where I used the provided STEP files and just measured the necessary sizes in fusion. 
The 1x7 headers for the ESP32 ARE spaced out correctly (Again, made sure by measuring the part in fusion), and I made sure the display had enough space so it wouldnt cover the switches. I also chose to do the esp32 on the back side, the display ontop, and the switches lined up below the display. I also added mounting holes and added a custom logo to my pcb just for fun (and mounting holes for mounting to case). Below is the PCB: 

![image](https://stasis.hackclub-assets.com/images/1772987073926-q4d94m.png)

In addition, I used the provided STEP files to make the 3D viewer. Just a note, the DIsplay does not have headers in this 3D viewer, because they are specially spaced pins (3mm apart not 2.54mm), so I will have 
to use the header pins which come with the display itself. I also made sure to adjust the footprint so that it was actually 3mm apart and would work. Below is the 3D model: 

![image](https://stasis.hackclub-assets.com/images/1772987264157-xksd8z.png)

Backside: 

![image](https://stasis.hackclub-assets.com/images/1772987291831-x2tzpi.png)

Next I saved the STEP for the board with the parts, and moved on over to fusion. I utilized the mounting holes and provided holes on each side of it, so I can use heat-set inserts on the bottom half, and then use screws to secure the top half and pcb into place. I chose to go with somewhat of a unique spotify display design, as I made it so the display was sideways and ontop, then the 3 switches below it, and it was like a stand proped up almost in the shape of an iphone holder. Below are some images of the 3D model: 

![image](https://stasis.hackclub-assets.com/images/1772987475771-anuowg.png)

![image](https://stasis.hackclub-assets.com/images/1772987487670-cg1l4k.png)

![image](https://stasis.hackclub-assets.com/images/1772987513792-gfvkky.png)

![image](https://stasis.hackclub-assets.com/images/1772987541316-9lp0tq.png)

After basically finishing the whole 3D modelling and such, I worked on the firmware, where although I cannot put here, I provided a snippet of the code below, with the full thing being in the github repo:

![image](https://stasis.hackclub-assets.com/images/1772987698248-z54bf8.png)

All files have been uploaded to the github repo, and just to be sure I had also provided the gerber files so its easier to just take them straight off of there when I order the pcb. 
Lastly, ECHO, or Embedded Controller & Hidden Operations (yes I know the name is weird, it'll make sense), is a desk Spotify display built using an ESP32 C3 Mini, alongside a 1.8" TFT screen, 3 mechanical switches for playback control, and a hidden Space Invaders game unlocked by holding all 3 keys (hence the "& Hidden Operations").
The display shows your current track with mood-based colors, scrolling text, animated bars, and a progress bar. The PCB was (although as expected) built in KiCad while the case was designed around the full PCB in Fusion360. 
The project includes a Case folder with CAD files, a PCB folder with all KiCad and manufacturing files, and Firmware folder with the Arduino firmware. 
I would also like to point out. The PCB was not needed for this project, however, I am still working on my PCB designing skills so I could use the challenge. Also, the firmware provided is NOT the final version, as I can only really finish it once I have the ESP32 and know its IP. 

Thanks for checking out ECHO. And don't forget to hold all 3 keys. 

![Adobe Express - file](https://stasis.hackclub-assets.com/images/1772988462683-rp2koo.png)

Github Repo Link: 
https://github.com/groberts822/ECHO-Display 

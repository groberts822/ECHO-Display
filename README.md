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

I would like to note, there are holes for heat-press inserts, which I have myself, alongside the proper screws, so unless it comes in the kit for this starter project, I dont need them hence why they aren't separately mentioned in the BOM. 

After basically finishing the whole 3D modelling and such, I worked on the firmware, where although I cannot put here, I provided a snippet of the code below, with the full thing being in the github repo:

![image](https://stasis.hackclub-assets.com/images/1772987698248-z54bf8.png)

BOM is below:

| Qty | Part | Link | Notes |
|-----|------|------|-------|
| 1 | ESP32 LOLIN C3 Mini | [AliExpress](https://www.aliexpress.us/item/3256804553736450.html?pdp_npi=4%40dis%21USD%21US%20%245.47%21US%20%240.99%21%21%215.47%210.99%21%402103119417736152975654869d1287%2112000056120453300%21sh%21US%217425461850%21X&spm=a2g0o.store_pc_home.productList_2009695634908.1005004740051202&gatewayAdapt=glo2usa) | |
| 1 | 1.8" TFT Display ST7735 | [AliExpress](https://www.aliexpress.us/item/3256809750593574.html?spm=a2g0o.productlist.main.2.71184dab1tsmCB&algo_pvid=4540c901-2267-4297-894a-94d7b7e40b90&algo_exp_id=4540c901-2267-4297-894a-94d7b7e40b90-1&pdp_ext_f=%7B"order"%3A"199"%2C"eval"%3A"1"%2C"fromPage"%3A"search"%7D&pdp_npi=6%40dis%21USD%218.14%210.99%21%21%2155.72%216.76%21%40210311cc17736153139033443ee5db%2112000050625051194%21sea%21US%217425461850%21ABX%211%210%21n_tag%3A-29910%3Bd%3A6670291b%3Bm03_new_user%3A-29895%3BpisId%3A5000000197847475&curPageLogUid=sl2faarfQKwY&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005009936908326%7C_p_origin_prod%3A&_gl=1*4nhmo6*_gcl_au*OTExMzQxMzAyLjE3NzM2MTUyNjg.*_ga*MTY2NjQwMzk4OC4xNzczNjE1MjY4*_ga_VED1YSGNC7*czE3NzM2MTUyNjgkbzEkZzEkdDE3NzM2MTUzMTIkajE2JGwwJGgw) | |
| 1 | Custom PCB | [JLCPCB](https://jlcpcb.com) | Already ordered |
| 3 | Cherry MX Switches | [NovelKeys](https://novelkeys.com/products/cherry-mx2a-switches) | Leftover from Hackpad |
| 3 | Keycaps (1u) | [NovelKeys](https://novelkeys.com) | Leftover from Hackpad |
| 4 | M3 Heatset Inserts | [Amazon](https://www.amazon.com/initeq-M3-0-5-Threaded-Inserts-Printing/dp/B077CJV3Z9) | Leftover from Hackpad |
| 4 | M3 Screws | [Amazon](https://www.amazon.com/s?k=m3+screws) | Leftover from Hackpad |

All files have been uploaded to the github repo, and just to be sure I had also provided the gerber files so its easier to just take them straight off of there when I order the pcb. 
Lastly, ECHO, or Embedded Controller & Hidden Operations (yes I know the name is weird, it'll make sense), is a desk Spotify display built using an ESP32 C3 Mini, alongside a 1.8" TFT screen, 3 mechanical switches for playback control, and a hidden Space Invaders game unlocked by holding all 3 keys (hence the "& Hidden Operations").
The display shows your current track with mood-based colors, scrolling text, animated bars, and a progress bar. The PCB was (although as expected) built in KiCad while the case was designed around the full PCB in Fusion360. 
The project includes a Case folder with CAD files, a PCB folder with all KiCad and manufacturing files, and Firmware folder with the Arduino firmware. 
I would also like to point out. The PCB was not needed for this project, however, I am still working on my PCB designing skills so I could use the challenge. Also, the firmware provided is NOT the final version, as I can only really finish it once I have the ESP32 and know its IP. 

Thanks for checking out ECHO. And don't forget to hold all 3 keys. 

![Adobe Express - file](https://stasis.hackclub-assets.com/images/1772988462683-rp2koo.png)

Github Repo Link: 
https://github.com/groberts822/ECHO-Display 

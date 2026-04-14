# Neon Oubliette - Art Direction & Asset List

## 🎨 The "Vibe" & Tone Guide

To keep the assets unified in Draw Things or other image generators, use a consistent base prompt and negative prompt for every generation, swapping only the subject.

**Global Base Prompt (Prefix):**
> `16-bit SNES style pixel art, true top-down perspective, cyberpunk dystopian dark sci-fi, crisp pixels, limited color palette, retro 2D game asset, flat lighting with neon accents, black background -- subject:`

**Global Negative Prompt:**
> `isometric, 3d, realistic, high resolution, blurry, messy pixels, jpeg artifacts, gradient shading, modern illustration, text, watermark`

**Color Palette:**
- **Slums/Industrial:** Rust browns, muted grays, sickly toxic greens.
- **Corporate/Core:** Sleek silvers, deep blacks, stark neon blues and magentas.
- **Vehicles:** Utilitarian grays with bright, glowing engine exhausts/hover-pads.

---

## 📋 Required Asset List

Here is the breakdown of the sprites we will need to replace our placeholder rectangles and geometry.

### 1. Characters & NPCs (Approx. 16x16 or 32x32 pixels)
*All characters should be seen from a top-down (birds-eye) perspective (head and shoulders visible).*
* **Player Character:** A distinct protagonist. *(Prompt: solitary street survivor in a glowing high-tech jacket)*
* **Pedestrians:** Generic citizens for the crowd systems. *(Prompt: huddled citizen in a hooded trenchcoat)*
* **Archon (Elite/Enforcer):** High-status authority figures. *(Prompt: elite futuristic peacekeeper with sleek chrome armor and a glowing visor)*
* **Aborrax (Threat):** The dangerous/alien elements of the city. *(Prompt: grotesque bio-mechanical creature, dark organic plating)*

### 2. Vehicles (Top-Down)
*Keep these oriented facing "North" (up) so we can rotate them via SDL2 in the code.*
* **EMMV (Micro-Mobility Pod):** Compact, cheap transport. *(Prompt: tiny boxy futuristic smart-car, rusted metal, glowing rear thruster)*
* **Maglift Car:** Standard civilian vehicle. *(Prompt: sleek hovering cyberpunk sedan, smooth aerodynamic curves, neon taillights)*
* **Maglift Transport:** The transit train cars (we link these together). *(Prompt: single long armored hover-train carriage, industrial sliding doors)*

### 3. Environment & Infrastructure (Tileable, e.g., 32x32 or 64x64 pixels)
* **Road Surfaces:** 
  * Primary/Secondary: *(Prompt: cracked dark asphalt road tile with faded neon lane markers)*
  * Pedestrian Alley: *(Prompt: narrow grimy cobblestone path tile, scattered trash, neon reflections)*
* **Traffic Lights:** A top-down gantry or small indicator. *(Prompt: futuristic traffic light pole, glowing [red/yellow/green] halo)*
* **Transit Station / Stop:** *(Prompt: glowing holographic bus stop shelter canopy, top-down roof view)*

### 4. Buildings & Zoning (Roof-top views)
*Since the game is top-down, building sprites will primarily be the rooftops and the immediate parallax/shadow dropping down to the street.*
* **URBAN_CORE:** *(Prompt: massive sprawling skyscraper rooftop, glowing corporate helipad, dense air conditioning vents, neon borders)*
* **SLUM:** *(Prompt: patchwork shanty town rooftop tile, corrugated rusted metal, exposed wires, glowing makeshift heaters)*
* **RESIDENTIAL:** *(Prompt: dense brutalist apartment complex rooftop, scattered solar panels, grim grey concrete)*
* **CORPORATE:** *(Prompt: sleek monolithic black glass building roof, minimalist, glowing blue data streams)*
* **INDUSTRIAL:** *(Prompt: heavily polluted factory roof, massive dark exhaust smokestacks, glowing molten vents)*

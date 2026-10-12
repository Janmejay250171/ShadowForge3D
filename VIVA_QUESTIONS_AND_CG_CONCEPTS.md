# SHADOWFORGE 3D — Computer Graphics Concepts & Viva Voce Guide

This document is prepared specifically for **viva voce, lab evaluations, and project defense**. 
It focuses on the **core conceptual principles of Computer Graphics** (how 3D geometry works, how shapes are built, how they are joined, how matrices transform space, and how shadows are formed and matched) in clear, intuitive terms suitable for oral explanation.

---

## Quick Navigation
1. [Core Concept of the Game](#1-core-concept-of-the-game)
2. [3D Modeling Concepts: How Shapes are Built](#2-3d-modeling-concepts-how-shapes-are-built)
3. [Joining Shapes into a Full Figure (The Big Concept)](#3-joining-shapes-into-a-full-figure-the-big-concept)
4. [Coordinate Systems & The Transformation Pipeline](#4-coordinate-systems--the-transformation-pipeline)
5. [Matrix Mathematics & Rotations (Conceptual)](#5-matrix-mathematics--rotations-conceptual)
6. [3D-to-2D Shadow / Silhouette Projection](#6-3d-to-2d-shadow--silhouette-projection)
7. [Silhouette Matching: Area Occupancy & IoU](#7-silhouette-matching-area-occupancy--iou)
8. [Lighting, Shading & Normal Vectors](#8-lighting-shading--normal-vectors)
9. [Viewport, Hidden Surface Removal & Display](#9-viewport-hidden-surface-removal--display)
10. [Top 25 Viva Voce Questions & Answers (Conceptual Focus)](#10-top-25-viva-voce-questions--answers-conceptual-focus)

---

## 1. Core Concept of the Game

### What is the fundamental Computer Graphics premise of ShadowForge 3D?
In 3D computer graphics, **a 3D object is a 3-dimensional volume, but a screen (or a shadow) is purely 2-dimensional**. 
- Whenever you project a 3D object onto a 2D surface, you discard one dimension of information (depth).
- Depending on the **orientation (angle of rotation)** of the 3D object relative to the light/camera, it casts completely different 2D planar silhouettes.
- **The Gameplay Loop:** The player is shown a target 2D silhouette on the right. On the left is an abstract 3D composite sculpture. By rotating the sculpture in 3D space around the $X$, $Y$, and $Z$ axes, the player searches for the exact spatial orientation that casts a 2D shadow matching the target shape ($\ge 95\%$ match).

---

## 2. 3D Modeling Concepts: How Shapes are Built

> **Viva Key Idea:** When the professor asks *"How did you build the shapes?"*, do not talk about code variables or file reading. Explain the **geometric surface representation**.

### A. Boundary Representation (B-Rep) / Polygonal Mesh
In computer graphics, a 3D solid object is not represented as solid matter; it is represented by its **hollow boundary surface (skin)**. 
This surface is constructed using three basic elements of geometry:
1. **Vertices:** Points defined by 3D coordinates $(x, y, z)$ in space.
2. **Edges:** Straight line segments connecting two vertices.
3. **Faces:** Flat planar surfaces enclosed by a loop of edges.

### B. Why Triangles are the Fundamental "Atom" of 3D Graphics
Every shape in our game is ultimately broken down into **triangles**. In a viva, professors love to ask: *"Why triangles? Why not squares or circles?"*
There are three fundamental mathematical reasons:
1. **Guaranteed Planarity:** Any 3 non-collinear points in space define **exactly one flat plane**. A 4-sided polygon (quad) can twist such that the fourth vertex is bent out of plane, creating an invalid non-planar surface. A triangle can *never* bend.
2. **Strict Convexity:** A triangle is always convex. Convex shapes are trivial for computer hardware to rasterize, interpolate, and test for point containment.
3. **Unique Normal Vector:** Three vertices determine a single unambiguous perpendicular normal vector ($\vec{N} = \vec{AB} \times \vec{AC}$), essential for correct lighting.

```
       Quad (Can bend & warp)                     Triangulated Quad (Always Flat)
           v1 ---------- v2                           v1 ---------- v2
           /              \                           /  \          /
          /   Non-planar   \                         /    \   T2   /
         /    distortion    \                       /  T1  \      /
        v0 ----------------- v3                    v0 ------\--- v3
                                                 (Diagonal split ensures planarity)
```

### C. Construction of the Geometric Primitives
1. **Box / Cuboid:**
   - Defined by an **origin center** $(c_x, c_y, c_z)$ and **dimensions** $(\text{width}, \text{height}, \text{depth})$.
   - It has **6 rectangular planar faces** (Front, Back, Top, Bottom, Left, Right).
   - Each face is formed by 4 corner vertices connected into a quad.
   - Each quad is split diagonally into **2 triangles**.
   - **Total:** $6 \text{ faces} \times 2 = 12 \text{ triangles}$ ($36$ vertices).

2. **Pyramid:**
   - Defined by a **flat rectangular base** at the bottom and an **elevated apex point** at the top.
   - The bottom base is 1 quad = **2 triangles**.
   - The sides are formed by connecting the base perimeter edges up to the apex, creating **4 triangular side walls**.
   - **Total:** $2 + 4 = 6 \text{ triangles}$ ($18$ vertices).

---

## 3. Joining Shapes into a Full Figure (The Big Concept)

> **Viva Key Idea:** When asked *"How did you join multiple shapes together to create the full figure?"*, explain the concepts of **Local Coordinate Frames, Spatial Offsets, Polygonal Aggregation, and Depth-Buffer Intersection**.

```
                           [Apex Pyramid]
                            (Offset: +Y)
                                 /\
                                /  \
     [-X Offset Arm]   +--------------------+   [+X Offset Arm]
     +-------------+   |   Central Trunk    |   +-------------+
     |   Box #2    |===|      Box #1        |===|   Box #3    |
     +-------------+   |   (At Origin)      |   +-------------+
                       +--------------------+
                       |  Offset Base Fin   |
                       |      Box #4        |
                       +--------------------+
                       
           ALL PARTS SHARE THE SAME LOCAL ORIGIN (0,0,0)
```

### Concept 1: The Shared Local Reference Frame (Local Coordinate Space)
- The entire composite sculpture possesses **one single pivot / center point**, defined at $(0, 0, 0)$.
- We do not create shapes independently and try to align them in world space later. Instead, **every sub-shape is defined relative to this shared origin**.

### Concept 2: Primitive Instancing with Spatial Offsets & Dimensions
To build an architectural monument (like *The Monolith* or *The Cross-Blade*):
- **Trunk (Pillar):** Placed at the center $(0, 0, 0)$ with a tall, slender volume.
- **Horizontal Wings / Arms:** Instantiated with positive or negative horizontal offsets (e.g., center offset $+0.42$ on the $X$-axis).
- **Crown / Apex:** Instantiated with a positive vertical offset (e.g., center offset $+0.50$ on the $Y$-axis).
- **Asymmetric Counter-Weights:** Placed with diagonal offsets in $-X$ and $+Z$ to give the sculpture 3-dimensional silhouette depth.

### Concept 3: Polygonal Aggregation (Single Compound Entity)
- Conceptually, joining shapes means taking the **geometric union of their surface meshes**.
- All triangles belonging to the central pillar, the side arms, and the top pyramid are collected together into **one unified geometric description**.
- Because they belong to the same model description, they form a **single rigid body**.

### Concept 4: Physical Intersections vs. Boolean CSG (Why Shapes Don't Need "Seams")
A professor might ask: *"Did you weld the seams between overlapping boxes, or do a Boolean Union?"*
- **Theoretical Explanation:** In computer graphics, there are two ways to combine shapes:
  1. **Constructive Solid Geometry (CSG) / Boolean Mesh Union:** Mathematically slicing triangles where they intersect, deleting interior geometry, and welding new boundary vertices. This is computationally slow and complex.
  2. **Interpenetrating Meshes with Z-Buffer Resolution (The Modern Real-Time Approach):** The sub-shapes are allowed to **physically overlap and penetrate each other's volumes**.
- **How it works without visual bugs:** When the graphics hardware rasterizes the figure, the **Depth Buffer (Z-Buffer)** automatically checks every pixel. If an arm box penetrates inside the central pillar, the interior hidden surfaces fail the depth test and are never drawn! Only the outer visible shell is rendered.

### Concept 5: Unified Rigid-Body Transformation
Because all joined parts share the same local origin:
- When a rotation matrix is applied, **all parts rotate together as one solid body**.
- A side arm stays locked at its relative offset from the trunk, orbiting gracefully around the central origin just like a real physical sculpture.

---

## 4. Coordinate Systems & The Transformation Pipeline

> **Viva Key Idea:** Understand **where** vertices live at each stage and **why** we transform them.

$$\text{Local Space} \xrightarrow{\mathbf{M}} \text{World Space} \xrightarrow{\mathbf{V}} \text{View Space} \xrightarrow{\mathbf{P}} \text{Clip Space} \xrightarrow{\div w} \text{NDC Space} \xrightarrow{\text{Viewport}} \text{Screen Space}$$

```
+------------------------------------------------------------------------------------+
| 1. LOCAL (OBJECT) SPACE                                                           |
|    - Coordinates relative to the sculpture's own center (0, 0, 0).                |
|    - Allows easy mathematical definition of boxes and pyramids.                   |
+------------------------------------------------------------------------------------+
                                      ↓  [Model Matrix: M = Ry * Rz * Rx (YZX Non-Locking Composition)]
+------------------------------------------------------------------------------------+
| 2. WORLD SPACE                                                                     |
|    - The global coordinate system of the virtual world.                            |
|    - The sculpture is rotated and oriented into its active puzzle position.        |
+------------------------------------------------------------------------------------+
                                      ↓  [View Matrix: Look-At Camera]
+------------------------------------------------------------------------------------+
| 3. VIEW / EYE / CAMERA SPACE                                                       |
|    - The camera is at the origin (0, 0, 0), looking down the -Z axis.              |
|    - All coordinates represent distance relative to the viewer's eye.              |
+------------------------------------------------------------------------------------+
                                      ↓  [Projection Matrix: Perspective Frustum]
+------------------------------------------------------------------------------------+
| 4. CLIP SPACE                                                                      |
|    - Geometry is in 4D homogeneous coordinates (x, y, z, w).                       |
|    - Polygons outside the camera's viewing cone (frustum) are clipped away.        |
+------------------------------------------------------------------------------------+
                                      ↓  [Perspective Division: divide by w]
+------------------------------------------------------------------------------------+
| 5. NORMALIZED DEVICE COORDINATES (NDC)                                             |
|    - A standard cube from [-1, -1, -1] to [+1, +1, +1].                            |
|    - Objects farther away become smaller (depth foreshortening).                   |
+------------------------------------------------------------------------------------+
                                      ↓  [Viewport Transformation: glViewport]
+------------------------------------------------------------------------------------+
| 6. SCREEN / WINDOW SPACE                                                           |
|    - Physical pixels on the computer monitor (e.g., 1280 x 720).                   |
+------------------------------------------------------------------------------------+
```

---

## 5. Matrix Mathematics & Rotations (Conceptual)

### A. Why 4x4 Matrices & Homogeneous Coordinates ($w$)?
- A standard 3D point is $(x, y, z)$. A $3 \times 3$ matrix can represent **Rotation** and **Scaling**, but **it cannot represent Translation (shifting position)** as a linear matrix multiplication.
- By adding a 4th dummy coordinate ($w=1$), we create **Homogeneous Coordinates** $(x, y, z, 1)$.
- In 4D homogeneous space, Translation becomes a linear matrix multiplication.
- This allows **Translation, Rotation, and Scaling to be multiplied together into a single master matrix**:
  $$M = T \cdot R \cdot S$$

### B. Meaning of $w = 1$ vs $w = 0$:
- **$w = 1$ represents a Point in Space:** It has a position, so when translated, its location changes.
- **$w = 0$ represents a Direction / Vector:** It only has orientation and magnitude. It does *not* have a location, so translation has zero effect on it.

### C. Elementary 3D Rotations (Euler Angles)
In 3D space, an object can rotate around three independent axes:
1. **Pitch (Rotation around X-axis):** Tilting forward or backward.
2. **Yaw (Rotation around Y-axis):** Swiveling left or right (like shaking your head).
3. **Roll (Rotation around Z-axis):** Tilting side to side (like a plane banking).

### D. Why Matrix Multiplication Order Matters (Non-Commutative)
- Matrix multiplication is **not commutative**:
  $$A \cdot B \neq B \cdot A$$
- In the real world, if you rotate a book $90^\circ$ around $X$ and then $90^\circ$ around $Y$, it ends up in a completely different orientation than if you rotated around $Y$ first and then $X$.
- In ShadowForge 3D, we maintain a strictly defined evaluation sequence:
  $$M = R_x \cdot R_y \cdot R_z$$
  (Rotating around Z, then Y, then X).

---

## 6. 3D-to-2D Shadow / Silhouette Projection

> **Viva Key Idea:** How does a 3D figure turn into a flat 2D shadow?

```
         3D Object                                    Flat 2D Projection Plane
       in Camera Space                                        (Z = 0)
         
            /\                                                 /\
           /  \       =======[ Project Vertices ]======>      /  \
          /____\             Drop Z-Coordinate               /____\
          |    |                                             |    |
          |____|                                             |____|
       (x, y, z)                                           (x, y, 0)
```

### The Projection Principle:
1. Every vertex $(x, y, z)$ of the rotated 3D sculpture is projected into the camera's view frustum.
2. After perspective division, the coordinates arrive in **Normalized Device Coordinates (NDC)** in the range $[-1, 1]$.
3. **Flattening into a Silhouette:**
   - The depth component ($Z$) is flattened to zero ($z = 0.0$).
   - The resulting $(x_{\text{ndc}}, y_{\text{ndc}})$ coordinates form a 2D planar polygon representing the exact outline/silhouette cast toward the projection surface.
4. **Why Shadows Change:** As the 3D sculpture rotates, different geometric faces turn towards the projection axis, altering the overlapping 2D profile.

---

## 7. Silhouette Matching: Area Occupancy & IoU

> **Viva Key Idea:** Why do we compare 2D shapes rather than 3D rotation angles?

### A. The Core Conceptual Reason:
- A novice game would check: `if (playerRotation == targetRotation) win;`
- **Why this is conceptually invalid in Computer Graphics:**
  1. **Symmetry:** Many 3D sculptures have symmetrical geometry. Rotating a symmetric cross or pillar by $180^\circ$ produces the exact same shadow!
  2. **Alternative Solutions:** The goal of the puzzle is to **forge the shadow**, not match an angle. Any 3D orientation that successfully casts the target shadow is a valid physical solution.
  3. Therefore, comparison must happen **in the 2D projected image domain**.

### B. The Jaccard Index / Intersection-over-Union (IoU)
We compare the similarity of two 2D shapes using the mathematical formula:

$$\text{Similarity } (\%) = \frac{\text{Area of Overlap (Intersection)}}{\text{Total Combined Area (Union)}} \times 100\% = \frac{|A \cap B|}{|A \cup B|} \times 100\%$$

```
   Target Silhouette (A)            Live Silhouette (B)
        +--------+                       +--------+
        |        |                       |        |
        |    +---|----+              +---|----+   |
        |    | A ∩ B  |              | A ∩ B  |   |
        +----|---+    |              +---|----+   |
             |        |                  |        |
             +--------+                  +--------+

   Intersection (A ∩ B) : Probes covered by BOTH silhouettes simultaneously
   Union (A ∪ B)        : Probes covered by EITHER silhouette
```

### C. Conceptual Process: Uniform Probe Sampling Grid
1. A uniform grid of **sampling probe points** ($34 \times 34 = 1156$ points) is placed over the 2D projection screen.
2. For every probe point, we ask: *"Does this point lie inside the silhouette?"*
   - Tested using the **Point-in-Triangle test** (cross-product edge functions): if a point lies on the inner side of all 3 edges of any triangle, it is inside the shape.
3. If a probe point is covered by both the target and the live shadow $\to$ it counts toward **Intersection**.
4. If a probe point is covered by either shadow $\to$ it counts toward **Union**.
5. When the ratio reaches **$\ge 95\%$**, the match is considered complete!

---

## 8. Lighting, Shading & Normal Vectors

### A. Surface Normal Concept:
- A **Surface Normal** ($\vec{N}$) is a unit vector pointing straight out, perpendicular to a flat surface.
- The computer needs normal vectors to calculate **how much light hits a surface**.
- If a surface directly faces the light source, it receives full brightness ($\vec{N} \cdot \vec{L} = 1$). If it is angled away, it receives less light ($\vec{N} \cdot \vec{L} < 1$).

### B. Dynamic Screen-Space Face Normals (`dFdx` / `dFdy`):
- Instead of manually storing normal vectors for every vertex, we compute them dynamically in the fragment shader:
  - `dFdx` and `dFdy` measure how the 3D surface position changes between neighboring screen pixels.
  - Taking their **cross product** gives the exact perpendicular normal vector for that polygon face!
  - This creates clean, crisp, modern **faceted flat shading (low-poly look)**.

### C. Lighting Rig Concept:
- **Key Light:** Primary bright light source from the top-right, creating highlights and strong shadows.
- **Fill Light:** A softer, dimmer light from the opposite side, ensuring shadowed areas don't turn completely black.
- **Ambient Light:** Simulates natural scattered environmental light that illuminates all surfaces equally.

---

## 9. Viewport, Hidden Surface Removal & Display

### A. Split-Screen Dual Viewport:
- The screen is divided into two logical windows:
  - **Left Viewport:** The 3D interactive studio showing the solid sculpture.
  - **Right Viewport:** The 2D projection screen showing the silhouettes.
- Controlled using `glViewport` (defining the pixel window) and `glScissor` (restricting drawing and screen clears to that specific half).

### B. Hidden Surface Removal (The Z-Buffer Algorithm):
- When rendering 3D shapes, closer polygons must block farther polygons.
- The **Z-Buffer (Depth Buffer)** stores the depth distance of every pixel.
- When drawing a pixel, the GPU checks: *"Is this new pixel closer to the camera than what was already drawn?"*
  - If **Yes**: It draws the pixel and updates the depth buffer.
  - If **No**: It discards the pixel (it is hidden behind an existing surface).
- This is why overlapping boxes in our composite sculpture look seamless without visual bugs!

### C. Double Buffering:
- The computer uses two framebuffers:
  - **Front Buffer:** The image currently being displayed on your monitor.
  - **Back Buffer:** The image the GPU is actively drawing into.
- Once the GPU finishes drawing the entire scene, the two buffers swap. This eliminates **screen tearing and flickering**.

---

## 10. Top 25 Viva Voce Questions & Answers (Conceptual Focus)

### Q1: Can you summarize ALL the Computer Graphics concepts used in this game in one answer?
**Ans:** 
> **The 30-Second Elevator Pitch:**
> *"ShadowForge 3D implements the complete end-to-end Computer Graphics pipeline from scratch. It incorporates **procedural 3D boundary-representation modeling** and **shape joining via shared local coordinate frames**, **custom 4x4 affine matrix transformations** using homogeneous coordinates ($w=1$), the **standard 3D-to-2D viewing/projection pipeline**, **planar shadow projection** by collapsing depth, **real-time geometric silhouette matching** using the Jaccard Index (Intersection over Union), **dynamic surface normal generation** via screen-space partial derivatives for faceted flat shading, **hidden surface removal** using the Z-buffer, and **multi-viewport split-screen rendering**."*

If the examiner asks you to elaborate on the concepts, explain these **8 Core Pillars**:
1. **3D Surface Modeling & Primitive Aggregation (B-Rep):** 
   - Solid shapes are represented as hollow polygonal surface meshes.
   - All geometry is decomposed into **triangles** because they guarantee planarity, convexity, and unique normal vectors.
   - Complex sculptures are assembled by joining primitives (boxes and pyramids) defined with spatial offsets relative to a **shared local origin $(0, 0, 0)$**.
2. **3D Affine Transformations & 4x4 Homogeneous Coordinates:**
   - 3D points are augmented to 4D homogeneous vectors $(x, y, z, w)$ with $w=1$ (points) and $w=0$ (vectors) so that translation can be expressed as a linear matrix multiplication.
   - Full 3-axis Euler rotation matrices ($R_x$ Pitch, $R_y$ Yaw, $R_z$ Roll) are multiplied non-commutatively ($M = R_x \cdot R_y \cdot R_z$) to rotate the entire compound figure as a single rigid body.
3. **The 3D-to-2D Graphics Viewing & Projection Pipeline:**
   - Coordinates transition through six spaces: $\text{Local} \to \text{World} \to \text{View} \to \text{Clip} \to \text{NDC} \to \text{Screen}$.
   - **Perspective Division ($\div w$):** Scales coordinates by distance to create realistic depth foreshortening.
4. **Planar Shadow / Silhouette Projection:**
   - The 3D model vertices are transformed into Normalized Device Coordinates (NDC), and the depth component ($Z$) is flattened to $0.0$, collapsing the 3D geometry into a flat 2D shadow footprint.
5. **Hidden Surface Removal (HSR) & The Depth Buffer (Z-Buffer):**
   - The Z-buffer algorithm determines visibility at the pixel level during rasterization.
   - When joining shapes, sub-primitives can physically penetrate each other without manual Boolean mesh cutting because the Z-buffer automatically culls interior hidden surfaces.
6. **Silhouette Evaluation & Area Occupancy (Jaccard Index / IoU):**
   - Instead of comparing 3D angles, the game evaluates geometric shadow area overlap using Intersection over Union (IoU).
   - Uniform 2D grid probe sampling ($34 \times 34 = 1156$ points) with 2D point-in-triangle edge tests evaluates live similarity against the target.
7. **Illumination Modeling & Dynamic Normal Vectors:**
   - Surface normals are computed dynamically in the fragment shader using screen-space partial derivatives (`dFdx` / `dFdy`), enabling faceted flat shading without per-vertex normal buffers.
   - Two-point directional lighting rig (Key Light + Fill Light) plus ambient lighting based on Lambertian diffuse reflection.
8. **Rasterization, Multi-Viewport & Framebuffer Pipeline:**
   - Split-screen rendering using `glViewport` and `glScissor` with `GL_SCISSOR_TEST` to independently render the 3D studio on the left and the 2D projection on the right.
   - Double buffering to eliminate screen tearing, and alpha blending for transparent UI overlays.

### Q2: What is the core Computer Graphics problem being demonstrated in this project?
**Ans:** The project demonstrates the **relationship between 3D geometry and 2D planar projection**. A 3D object has infinite 2D shadow projections depending on its orientation relative to a projection plane. The game challenges the player to find the spatial 3D orientation that matches a specific 2D shadow.

### Q3: How are 3D solid shapes represented in computer graphics?
**Ans:** Through **Boundary Representation (B-Rep)** using a **polygonal mesh**. Solid volumes are represented by their outer surface boundary, made of **Vertices** (points in 3D), **Edges** (lines connecting vertices), and **Faces** (flat surfaces enclosed by edges).

### Q4: Why is the triangle the fundamental building block of 3D graphics?
**Ans:** 
1. Any 3 non-collinear points in space define **exactly one flat plane** (guaranteed planarity). Quads can warp or bend.
2. Triangles are always strictly **convex**, making rasterization and inside-outside testing mathematically foolproof.
3. They possess a single, unambiguous surface normal vector.

### Q4: How is a 3D box/cuboid built conceptually?
**Ans:** A box consists of 6 planar rectangular faces. Each face is defined by 4 corner vertices forming a quad. Each quad is split diagonally into 2 triangles. Therefore, a complete box is made of $6 \times 2 = 12$ triangles.

### Q5: How is a 3D pyramid built conceptually?
**Ans:** A pyramid consists of 4 base vertices forming a flat square bottom (2 triangles), plus an elevated apex vertex. Four triangular side walls slope upwards from each base edge to meet at the apex ($2 + 4 = 6$ triangles total).

### Q6: Conceptually, how do you join multiple shapes into a single full figure?
**Ans:** 
1. All individual shapes share a **common local coordinate system** centered at a single pivot origin $(0, 0, 0)$.
2. Each sub-shape (pillar, side arms, crown pyramid) is positioned at a specific **spatial offset** and dimension relative to that common origin.
3. All the triangles of all parts are aggregated together into a single surface model.
4. Because they share the same coordinate frame, applying a rotation matrix rotates the entire composite figure as a single solid rigid body.

### Q7: What happens where shapes physically overlap or intersect? Did you merge the seams?
**Ans:** No, we do not perform complex geometric seam cutting (CSG Boolean Union). In real-time computer graphics, primitives are allowed to physically interpenetrate. The **Z-buffer (Depth Test)** automatically resolves visibility at the pixel level during rasterization, naturally hiding any geometry buried inside another shape.

### Q8: What are Homogeneous Coordinates, and why do we use 4D vectors $(x, y, z, w)$ for 3D graphics?
**Ans:** In 3D space, translation is an addition, which cannot be represented as a $3 \times 3$ matrix multiplication. By adding a 4th coordinate ($w=1$), translation becomes a linear matrix multiplication. This allows Translation, Rotation, and Scaling to be combined into a single $4 \times 4$ composite transformation matrix.

### Q9: What is the conceptual difference between $w=1$ and $w=0$?
**Ans:**
- $w = 1$ represents a **Point in Space** (has a position, affected by translation).
- $w = 0$ represents a **Direction or Vector** (no position, unaffected by translation, only rotates and scales).

### Q10: What are Euler Angles, and which ones are used in the project?
**Ans:** Euler angles describe orientation using three sequential rotations around cardinal axes:
- **Pitch:** Rotation around the $X$-axis.
- **Yaw:** Rotation around the $Y$-axis.
- **Roll:** Rotation around the $Z$-axis.

### Q11: Why is matrix multiplication order important?
**Ans:** Matrix multiplication is **non-commutative** ($A \cdot B \neq B \cdot A$). Rotating around the $X$-axis then the $Y$-axis yields a completely different spatial orientation than rotating around $Y$ first and then $X$.

### Q12: Explain the stages of the Transformation Pipeline in simple terms.
**Ans:**
1. **Local Space:** Coordinates defined relative to the object's own center.
2. **World Space:** The object positioned and oriented in the global virtual scene.
3. **View Space:** Scene transformed so the camera is at $(0, 0, 0)$ looking forward.
4. **Clip Space:** Perspective applied; objects outside the camera's viewing cone are clipped.
5. **NDC (Normalized Device Coordinates):** Perspective division $(\div w)$ scales geometry into a standard $[-1, 1]$ cube.
6. **Screen Space:** Coordinates mapped to physical monitor pixels.

### Q13: What is Perspective Division and why does it occur?
**Ans:** It is the mathematical operation of dividing the $(x, y, z)$ clip coordinates by the homogeneous component $w$. Because $w$ is proportional to the distance from the camera, dividing by $w$ causes distant objects to shrink, producing the realistic illusion of perspective depth.

### Q14: How is the 2D shadow projection produced from the 3D sculpture?
**Ans:** The 3D vertices are transformed through the camera and projection matrices into Normalized Device Coordinates. The depth dimension ($Z$) is then flattened to $0$, collapsing the 3D volume into a flat 2D silhouette on the projection plane.

### Q15: Why does the game match 2D silhouettes instead of comparing rotation angles?
**Ans:** Because 3D objects with symmetries can cast identical 2D shadows from multiple different rotation angles. The objective is to reproduce the *shadow*, so any 3D rotation that produces the matching silhouette is a valid, correct solution.

### Q16: What mathematical metric is used to evaluate silhouette similarity?
**Ans:** The **Jaccard Index**, also known as **Intersection over Union (IoU)**:
$$\text{Similarity } = \frac{\text{Area}(A \cap B)}{\text{Area}(A \cup B)} \times 100\%$$
It calculates the ratio of overlapping silhouette area to the total combined silhouette area. A score $\ge 95\%$ signifies a successful match.

### Q17: How is area overlap computed conceptually in real time?
**Ans:** We place a uniform grid of sampling probe points across the 2D canvas. For each point, we run a **Point-in-Triangle test** against both the live and target silhouettes. Counting how many probe points fall inside both shapes (intersection) versus either shape (union) yields the percentage match.

### Q18: How does the Point-in-Triangle test work conceptually?
**Ans:** Using 2D cross-product edge functions (barycentric coordinates). If a point lies on the positive (inner) half-plane of all three directed edges of a triangle, it is confirmed to be inside that triangle.

### Q19: What is a Surface Normal, and why is it needed?
**Ans:** A surface normal is a perpendicular unit vector pointing outwards from a surface. It is essential for lighting calculations to determine the angle at which light rays strike the polygon.

### Q20: How do you achieve faceted flat shading without storing normal vectors?
**Ans:** Using screen-space partial derivatives (`dFdx` and `dFdy`) in the fragment shader. They measure the rate of change of pixel positions along the screen $X$ and $Y$ directions; their cross product calculates the exact geometric surface normal of the polygon face.

### Q21: What is the purpose of the Z-Buffer (Depth Buffer)?
**Ans:** It solves the **Hidden Surface Removal (HSR)** problem. It records the depth of the closest surface at each pixel. When a new polygon fragment arrives, it is only drawn if its depth is closer than the existing depth in the buffer.

### Q22: Why is depth testing disabled when drawing the 2D shadow and UI?
**Ans:** Because the 2D shadow and HUD are flat overlays that should not be obscured or clipped by 3D depth calculations. They are rendered in sequential painter's order on top of the scene.

### Q23: How do you create a split screen showing 3D on the left and 2D on the right?
**Ans:** Using `glViewport` and `glScissor`. `glViewport` defines the mapping to screen pixels for each half, while `glScissor` confines screen clearing and rasterization to that specific half of the display.

### Q24: What is Double Buffering and why is it used?
**Ans:** It maintains two image buffers: a Front Buffer displayed to the monitor and a Back Buffer where the GPU renders the next frame. Swapping them instantly when rendering completes prevents visual flickering and screen tearing.

### Q25: What is Alpha Blending?
**Ans:** It is the mathematical blending of transparent foreground pixels with background pixels:
$$C_{\text{final}} = C_{\text{src}} \cdot \alpha + C_{\text{dst}} \cdot (1 - \alpha)$$
Used to render translucent reference silhouettes, UI frames, and HUD overlays.

### Q26: What is Gimbal Lock, and how do you ensure the rotation axes remain independent during gameplay?
**Ans:** **Gimbal Lock** occurs in Euler angle representations when two rotation axes align and become collinear (parallel), causing the loss of one degree of freedom (meaning rotating on either axis produces the exact same motion). In our game, using the **$YZX$ matrix multiplication order** ($\mathbf{M} = \mathbf{R}_y \cdot \mathbf{R}_z \cdot \mathbf{R}_x$) guarantees that the Yaw axis ($W/S$, rotating around world $[0, 1, 0]$) and Roll axis ($Q/E$, rotating around camera line of sight) have a dot product of identically zero ($\vec{u}_Y \cdot \vec{u}_Z = 0$), mathematically guaranteeing they remain strictly orthogonal and never collapse into each other across all gameplay orientations.

---

## Quick Viva Summary Matrix

| Question Area | Core Concept to Speak | What to Avoid Saying |
|---|---|---|
| **Shape Building** | Boundary representation, polygonal mesh, triangulated primitives (guaranteed flat & convex) | "I made a C++ struct with floats and vectors" |
| **Joining Shapes** | Shared local origin $(0, 0, 0)$, spatial offset translation, compound rigid body, Z-buffer handles overlap | "I just pushed more vertices into the list" |
| **4x4 Matrices** | Homogeneous coordinates ($w=1$), allows translation as matrix multiplication, composite $T \cdot R \cdot S$ | "It's just an array of 16 numbers" |
| **Gimbal Lock & Axes** | Gimbal lock causes axis alignment; solved using $YZX$ order keeping Yaw and Roll strictly orthogonal | "It was a random keyboard glitch" |
| **Shadow Projection** | Planar geometric projection, discarding depth ($Z=0$) in Normalized Device Coordinates | "I took a screenshot of the model" |
| **Silhouette Match** | Jaccard Index (Intersection over Union), 2D probe grid, evaluates shadow shape (not rotation angles) | "I checked if player angle equals target angle" |
| **Flat Shading** | Screen-space partial derivatives (`dFdx`/`dFdy`) computing dynamic perpendicular face normals | "I manually hardcoded normals for all 6 faces" |
| **Screen Split** | Dual viewports via `glViewport` and `glScissor` partition | "I opened two windows" |

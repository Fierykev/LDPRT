**<p align="center">Local Deformable Precomputed Radiance Transfer</p>**

<p align="center">(Note: all photos are taken from the project and run at approximately
40FPS with AA and greater than 60 FPS without AA on a GTX 970M)</p>

<p align="center"><img src="media/image1.png" width="400" height="400" /></p>

**<p align="center">Motivation</p>**

Local Deformable Precomputed Radiance Transfer (LDPRT) is a method
proposed by Microsoft in 2005 to compensate for the common problems with
its predecessor: Precomputed Radiance Transfer (PRT). PRTs permit an
application to represent diffuse lighting phenomena such as shadows,
interreflections, and subsurface scattering as a set of constants. The
constants are calculated though an offline simulation via methods
similar to that of a raytracer. To accomplish this feat, PRTs represent
data in the scene using a Spherical Harmonic basis. Because Spherical Harmonics
form the basis of SO(3) (3D rotation group), information for how
lighting interacts at arbitrary light angles can be stored within them.
Once the constants are produced, models can be rendered in real-time
with indirect illumination effects. These effects are consistent for all
camera positions given the light source is infinitely far away from the
scene. The direction of the incoming light is input into the PRT at
runtime and vertex colors taking into account objects in the scene are
returned. Unfortunately, PRTs only work for static scenes as when a
vertex position changes, the PRT lighting model fails. LDPRTs seek to
compensate for PRTs Achilles heel by using a Zonal Harmonic basis. Thus,
an LDPRT can quickly rotate a rest coordinate frame to its current
deformed orientation. LDPRTs work best for limited motion such as facial
expressions as they do not fully model global illumination for the
rotated frame.  Surprisingly, LDPRTs do not need to store any more data than
PRTs (assuming one lobe axis) and can be evaluated in constant time per vertex.
For the purposes of this project, a 6th order Spherical Harmonic is evaluated, 
leading to 36 coefficients for both the PRT and LDPRT.

**<p align="center">Morph Targeting</p>**

Morph Targeting is an alternative to Skeletal Animation.  Morph Targeting allows for an artist to move vertices on a model to a desired position in order to more effectively create the ideal pose.  Once all poses / morphs are made by the artist, a base position (a chosen pose) is established.  To animate the figure, the artist interpolates between the difference between the morph vertex's position and that of the base.  Morph Targets are primarily used in facial animation to give the artist complete control over how a character looks.  The primary downside to Morph Targeting is that it is a much more cumbersome job for an artist than rigging a bone structure than Skeletal Animation.  However, the result shows more polish in the detail of the animation key frames.

<p align="center">Left side is the Base Position (no morph applied); Right side is a Morph Target that alters vertex positions:</p>

<p align="center"><img src="media/image2.png" width="300" height="300" /><img src="media/image3.png" width="300" height="300" /></p>
<p align="center"><img src="media/image12.png" width="300" height="300" /><img src="media/image13.png" width="300" height="300" /></p>

**<p align="center">Wrinkle Model</p>**

A wrinkle model was also implemented to alter normals based on the
difference in primitive area between the rest and deformed frame.
Normals are created by linlearly interpolating between deformed frame
normals and bump map normal. The result is that deforming skin leads to
darker wrinkles. To increase the prominence of wrinkles and small
details, the mesh is tessellated with a displacement map applied to it.
A displacement map and bump map are both utilized because the
displacement map operates at a coarser grain than the bump map, leading
to a good mix of detail at interactive framerates.

<p align="center"><img src="media/image4.png" width="300" height="300" /></p>

**<p align="center">Optimization of Lobe Axis</p>**

Using Zonal Harmonics, LDPRTs estimate PRT lighting with a BFGS to
minimize error between the two models. Although Zonal Harmonics can have
multiple lobe axis, one is chosen for this project to reduce the amount
of data needing to be transferred to the GPU. Zonal Harmonics require a
lobe axis for rotation which, in most cases, is the same as the normal.
Hence, after the BFGS is completed, model normals are replaced with the
direction of Zonal Harmonic lobe axis.  Since the BFGS algorithm needs the
gradient of the objective variables, Mathematica was used to compute the
gradient of the 6th degree Spherical Harmonic.

<p align="center">(approximately .01 difference for single lobe which is not noticeable)</p>

<p align="center"><img src="media/image5.png" width="300" height="300" /><img src="media/image6.png" width="300" height="300" /></p>

**<p align="center">Soft shadows</p>**

To compute a PRT, several samples are taken for light sources scattered around a sphere
that has an infinite radius.  Since the light source is infinitely far away, a ray can
be cast from every vertex on the mesh towards the light source to see if it is blocked. 
Thus, for each sample light source, one ray is produced for every vertex per sample.  
Because this is similar to Monte Carlo integration in a ray tracer for soft shadows,
soft shadows are obtained with no additional cost to what the PRT would already have
to do to light the scene.

<p align="center"><img src="media/image10.png" width="400" height="400" /></p>

**<p align="center">Interreflections</p>**

The results from the soft shadow PRT yield the amount of energy present
at each vertex in the scene. To preform radiosity, all surfaces are
assumed to be lambertian reflectors. For each bounce, seperate coefficients are used
and eventually summed together after a constant number of bounces is reached. The below scene
demonstrates interreflections in the infamous Cornell Box. The green and
red color of the walls “bleed” onto the white boxes in the scene.

<p align="center">Left side is green color bleed; Right side is red color bleed:</p>

<p align="center"><img src="media/image7.png" width="300" height="300" /><img src="media/image8.png" width="300" height="300" /></p>

**<p align="center">Subsurface Scattering</p>**

For subsurface scattering, the random walk algorithm is implemented. By
choosing several randomly generated paths, the subsurface scattering of
skin is approximated. After moving a set distance, the path is randomly
rotated by a certain number of degrees before proceeding.  Unfortunately,
the effects of subsurface scattering are hard to see since random walk
converges slowly.  Since PRTs must cast millions of rays just for shadows,
it is not feasible to produce enough rays for random walk to yield the
correct results.

<p align="center">Note how the area below the cheek is lit although the light source is on the other side of the model.</p>
<p align="center"><img src=media/image9.png width="300" height="300" /></p>

**<p align="center">Artist Defined Albedo</p>**

To give artists more control over the process, an 
artist can define the thickness of a texture via an albedo map.  The map's
RGB channels are interpreted as the albedo color while the alpha channel is
the thickness.  By applying a backlight whose intensity is effected by thickness
and color of the albedo texture, a lighting scheme taking into account membrane thickness is achieved.

<p align="center">Note how the portion of the model that is front lit displays a dark wood texture,
whereas the portion that is backlit uses a leopard pattern.  The lighting implies some leopard lining inside
of the mesh.</p>
<p align="center"><img src="media/image14.png" width="400" height="400" /></p>

**<p align="center">Libraries</p>**

OpenGL - for Rendering

freeglut - for Windowing and Keyboard Input

glew - for shader loading

libLBFGS - for optimization of LDPRT coefficients and lobe axis

DevIL - for image loading

**<p align="center">References for Information</p>**

https://www.microsoft.com/en-us/research/wp-content/uploads/2017/01/ldprt.pdf

https://graphics.cg.uni-saarland.de/fileadmin/cguds/courses/ss15/ris/slides/RIS18Kautz.pdf

http://silviojemma.com/public/papers/lighting/spherical-harmonic-lighting.pdf

http://onlinelibrary.wiley.com/doi/10.1111/j.1467-8659.2007.01071.x/pdf

http://www.inf.ufrgs.br/~oliveira/pubs_files/Slomp_Oliveira_Patricio-Tutorial-PRT.pdf

http://www.ppsloan.org/publications/StupidSH36.pdf

https://en.wikipedia.org/wiki/Spherical_harmonics#Condon.E2.80.93Shortley_phase

https://graphics.stanford.edu/papers/scatteringeqns/setalk.pdf

https://www.solidangle.com/research/s2013_bssrdf_slides.pdf

**<p align="center">References for Models</p>**
http://graphics.cs.williams.edu/data/meshes.xml

https://graphics.stanford.edu/data/3Dscanrep/

https://www.blender.org/

**<p align="center">Artifact</p>**

<p align="center"><img src="media/image11.png" /></p>

Hey ladies, he's single!!!!

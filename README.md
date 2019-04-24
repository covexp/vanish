# vanish

*Remove transient objects from an image sequence*

## Objective

Input: image sequence with a stable background and moving foreground objects

Output: single image of the background with transient details removed

![image sequence](http://covex.info/images/east_imperial_anim.gif)

![reconstructed background](http://covex.info/images/output.png)

The processing works on a pixel-by-pixel basis by classifying pixels in each input color channel into buckets, and then finding the mode–the biggest bucket–for each pixel. The final image is then reconstructed from the original pixel values, using the mode bucket to either accept or reject any given frame.

Bucketing is used because small movements, lighting variations and sensor noise cause minor variations in intensity at any given pixel even in stationary features. In places where pixel intensities cluster around a bucket boundary, there is an issue with the entries potentially being split across two buckets. Adding an extra set of buckets, offset by half the bucket size, takes care of this splitting problem.

## Dependencies

CImg (http://cimg.eu/)

Boost (http://www.boost.org/)

## Usage

<pre>
vanish [options]

Allowed options:
  --help                show help message
  --dir arg (=./input/) directory of input image sequence
  --type arg (=png)     file extension
  --bucket arg (=8)     bucket size
  --depth arg (=8)      channel bit depth
  --refine arg (=0)     number of refinement steps
  --samples arg (=64)   number of samples for bad frame detection

Example:  vanish --dir east_imperial/ --type tif --bucket 16
</pre>

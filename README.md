# vanish

Remove transient objects from an image sequence.

## Objective

*vanish* can be used to generate a single frame from an image sequence, where transient foreground objects (such as passerby, moving cars, flying birds, etc.) are removed and only the stable background is retained.

***Input***

Image sequence with a stable background and moving foreground objects.

***Output***

Single image of the background with transient details removed.

![image sequence](http://covex.info/images/east_imperial_anim.gif)

![reconstructed background](http://covex.info/images/output.png)

The processing works on a pixel-by-pixel basis by classifying pixels in each input color channel into buckets, and then finding the mode–the biggest bucket–for each pixel. The final image is then reconstructed from the original pixel values, using the mode bucket to either accept or reject any given frame.

Bucketing is used because small movements, lighting variations and sensor noise cause minor variations in intensity at any given pixel even in stationary features. In places where pixel intensities cluster around a bucket boundary, there is an issue with the entries potentially being split across two buckets. Adding an extra set of buckets, offset by half the bucket size, takes care of this splitting problem.

## Compatibiltity

Tested on Windows and Linux.

Using C++17.

## Dependencies

CImg (http://cimg.eu/)

cxxopts (https://github.com/jarro2783/cxxopts)

## Usage

<pre>
Usage:
  Vanish [OPTION...]

 default options:
      --help         show help message
      --dir arg      directory of input image sequence
      --type arg     file extension
      --bucket arg   bucket size (default: 8)
      --depth arg    channel bit depth
      --samples arg  number of samples for bad frame detection
      --conf arg     confidence level [0.0, 1.0] (default: 0.200000)
</pre>

## Improvements

Currently *vanish* doesn't do any processing to correct misaligned frames in the sequence, and relies on either a stable photography process, or a separate preprocessing pass using softare such as *align_image_stack* from the [Hugin Project](http://hugin.sourceforge.net/download/).

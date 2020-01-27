# background-subtraction

Background subtraction is a major preprocessing steps in many vision based applications. For example, consider the cases like visitor counter where a static camera takes the number of visitors entering or leaving the room, or a traffic camera extracting information about the vehicles etc. In all these cases, first you need to extract the person or vehicles alone. Technically, you need to extract the moving foreground from static background

 ## Basic methods
 
The basic methods have several limitations:
- They are based on the history of the single pixel;
- They don't use spatial information;
- The thresholds and parameters are difficult to to choose.

The methods applied in this simple example are:
- **Previous frame**: the background estimated at step 'n' is the image analyzed in step 'n-1' <br>
![equation](http://latex.codecogs.com/gif.latex?B%5Ctextsubscript%7Bn%7D%20%3DB%5Ctextsubscript%7Bn%7D%20-1)
- **Moving average**: the background is estimated as the average of the last 'k' frames ('k' determines how much
quickly the background adapts to changes) <br>
![equation](http://latex.codecogs.com/gif.latex?B%5Ctextsubscript%7Bn%7D%20%3D%20%5Cfrac%7B1%7D%7Bk%7D%20%5Csum_%7Bi%3Dn-k-2%7D%5E%7Bn-1%7D%20I%5Ctextsubscript%7Bi%7D)
- **Exponential moving average**: the alpha parameter determines the speed of background update. <br>
	![equation](http://latex.codecogs.com/gif.latex?B%5Ctextsubscript%7Bn&plus;1%7D%20%3D%20%5Calpha%20B%5Ctextsubscript%7Bn%7D%20&plus;%20%281-%5Calpha%29I%5Ctextsubscript%7Bn%7D)
	
 ## Prerequisites
 
 	sudo apt-get install libopencv-dev python3-opencv
	bash install-opencv.sh
	
 ## Results 
 **Previous frame**:\
 ![](result/prevfrem.gif) <br>
 **Moving average**:\
 ![](result/mov.gif)<br>
 **Exponential moving average**:\
 ![](result/exp.gif)

#!/usr/bin/python

from PIL import Image
import time
import RPi.GPIO as GPIO



# initalize GPIO

GPIO.setmode(GPIO.BCM)

GPIO.setup(11, GPIO.OUT)
GPIO.setup(5, GPIO.OUT)  #Enable
GPIO.setup(6, GPIO.OUT)  #Bit 0 Red
GPIO.setup(12, GPIO.OUT) #Bit 1 Red
GPIO.setup(13, GPIO.OUT) #Bit 2 Red
GPIO.setup(16, GPIO.OUT) #Bit 0 Green
GPIO.setup(19, GPIO.OUT) #Bit 1 Green
GPIO.setup(20, GPIO.OUT) #Bit 2 Green
GPIO.setup(21, GPIO.OUT) #Bit 0 Blue
GPIO.setup(26, GPIO.OUT) #Bit 1 Blue

pixelNum = 144

GPIO.output(11, 0)


print("Please type the file name of the image you would like to create: ")
imageFile = input()



# grab image and convert it into 144 RGB data
#imageFile = "logoBlack.png"
original = Image.open(imageFile)
# adjust width and height to your needs
width, height = original.size
scale = height/144.0
width = int(width /scale)

#width = 144
height = 144

# resive original file to fit 144 pixels
shrink = original.resize((width, height), Image.ANTIALIAS)    # best down-sizing filter
ext = ".png"

shrink = shrink.rotate(180)

# DEBUG save shrunk file
#shrink.save("ANTIALIAS" + ext)

# convert to get RGB pixel data
shrinkRGB = shrink.convert('RGB')
color = ()
pixels = []
row = width
col = height

# iterate over all pixels 
#for i in range(0,row):
for i in range (0,row):									# to only get a the first row of the image, which should be the first column of the original 
	for j in range(0,col):
		r, g, b = shrinkRGB.getpixel((i, j))			# SWAP i and j to get picture transposed, may be needed for verticle pixel drawing
		r = r//36
		g = g//36
		b = b//85
		color = (r, g, b)
		#color = (r*36, g*36, b*85)						# UNCOMMENT to reconstruct, otherwise PIC should do this
		pixels.append(color) 


img = Image.new('RGB', (height, width))
img.putdata(pixels)
#img = img.transpose(Image.TRANSPOSE)
#img.save('recon.png')

#for i in range(0,pixelNum):
#    if(i%2):
#        color = (255//36, 0, 0)
#    else:
#        color = (0, 255//36, 0)
#    pixels[i] = color
#end for
k = 0

num = pixelNum-2
try:
	while True:
		GPIO.output(11,1)

		for j in range(0,4):
			print(j)
			for i in range(0, num):
				GPIO.output(6, 1)
				GPIO.output(12, 1)
				GPIO.output(13, 1)
				GPIO.output(16, 1)
				GPIO.output(19, 1)
				GPIO.output(20, 1)
				GPIO.output(21, 1)
				GPIO.output(26, 1)
				GPIO.output(5, 1)
				GPIO.output(5, 0)
			for i in range(num, 144):
				GPIO.output(6, 0)
				GPIO.output(12, 0)
				GPIO.output(13, 0)
				GPIO.output(16, 0)
				GPIO.output(19, 0)
				GPIO.output(20, 0)
				GPIO.output(21, 0)
				GPIO.output(26, 0)
				GPIO.output(5, 1)
				GPIO.output(5, 0)
			num = num - (144)//3
			time.sleep(1)


		for k in range(0,width):
			print(k)
			for i in range(0,pixelNum):
				#print(i)
				n = k*144
				GPIO.output(6, int(format(pixels[i+n][0],'#05b')[4])) #r0
				GPIO.output(12, int(format(pixels[i+n][0],'#05b')[3])) #r1
				GPIO.output(13, int(format(pixels[i+n][0],'#05b')[2])) #r2
				#GPIO.output(6, i%2) #g0
				#GPIO.output(12, i%2) #g1
				#GPIO.output(13, i%2) #g2
				GPIO.output(16, int(format(pixels[i+n][1],'#05b')[4])) #g0
				GPIO.output(19, int(format(pixels[i+n][1],'#05b')[3])) #g1
				GPIO.output(20, int(format(pixels[i+n][1],'#05b')[2])) #g2
				GPIO.output(21, int(format(pixels[i+n][2],'#04b')[3])) #b0
				GPIO.output(26, int(format(pixels[i+n][2],'#04b')[2])) #b1
				GPIO.output(5, GPIO.HIGH) #enable high
				#time.sleep(.1)
				GPIO.output(5, GPIO.LOW) #enable low
				#time.sleep(5)
			#end for
			#time.sleep(.05)
		#end for
		GPIO.output(11,0)
		time.sleep(5)
	#end while
except KeyboardInterrupt:
    GPIO.cleanup()
#end try


# reconstruct image from pixel data
recon = Image.new('RGB', (width, height))
recon.putdata(pixels)
#recon = recon.transpose(Image.TRANSPOSE)				# MAY NEED TO BE UNCOMMENTED
recon.save('recon.png')

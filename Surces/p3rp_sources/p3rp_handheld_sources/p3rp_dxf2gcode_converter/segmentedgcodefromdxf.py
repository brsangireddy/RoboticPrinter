import os
import sys
import time
openfile = sys.argv[1]

########## Segmentation Dimensions ###############
seglen = 18
segwidth = 12
layoutxmax = 108 #sys.argv[2]
layoutymax = 72 #sys.argv[3]
xsegnum = 0
ysegnum = 0
file_format = '00'
xsegstart = 0
ysegstart = 0
xsegmax = layoutxmax //seglen
ysegmax = layoutymax // segwidth
#print(xsegmax,ysegmax)


########## gcode variable ##################
Gcode = ""
gcodefilecount = 0

#LINE Buffer and Varibles
QTY         = 10000000
XMAX = 0
XMIN = 0
YMAX = 0
YMIN = 0
XSTART = [0 for i in range(QTY)]
YSTART = [0 for i in range(QTY)]
XEND   = [0 for i in range(QTY)]
YEND   = [0 for i in range(QTY)]
LINECOUNT	= 1
LCount = 1

DXFPATH, DXFNAME = os.path.split(openfile)

#### Read DXF file ############################################

file = open(openfile,errors='ignore')
while True: #### Main DXF read loop ####
	
	TEXT = file.readline()
	TEXT = TEXT.strip() # Remove spaces 
	#print(TEXT)
	############## LINE #####################
	if (TEXT == "AcDbLine"): # Found line
		#LINECOUNT = 1
		while True:
			TEXT = file.readline() # Read identifier
			TEXT = TEXT.strip()
			#print(TEXT)
			if (TEXT == "10"): 
				TEXT = file.readline() # Read X start poosition
				TEXT = TEXT.strip()
				XSTART[LINECOUNT]= float(TEXT)
				TEXT="NOTHING"
			
			if (TEXT == "20"): 
				TEXT = file.readline() # Read Y start poosition
				TEXT = TEXT.strip()
				YSTART[LINECOUNT]= float(TEXT)
				TEXT = "NOTHING"

			if (TEXT == "11"): 
				TEXT = file.readline() # Read X end poosition
				TEXT = TEXT.strip()
				XEND[LINECOUNT]= float(TEXT)
				TEXT = "NOTHING"
			
			if (TEXT == "21"): 
				TEXT = file.readline() # Read Y end poosition
				TEXT = TEXT.strip()
				#print("..........................LINE..........",TEXT)
				YEND[LINECOUNT]= float(TEXT)
				TEXT = "NOTHING"
			
			if (TEXT == "0"): # No more data
				if (XSTART[LINECOUNT] > XMAX): XMAX = XSTART[LINECOUNT]
				if (YSTART[LINECOUNT] < YMIN): YMIN = YSTART[LINECOUNT]
				if (YEND[LINECOUNT] > YMAX): YMAX = YEND[LINECOUNT]
				if (XEND[LINECOUNT] < XMIN): XMIN = XEND[LINECOUNT]
				
				LINECOUNT = LINECOUNT + 1
				break
		
	if( TEXT == "EOF"):
		break
file.close()
print("file reading is completed",LINECOUNT)

#for i in range (1,LINECOUNT):
#	print("XSATRT :", XSTART[i],",YSTART: ",YSTART[i],"                    XEND:",XEND[i],",YEND:",YEND[i])


for xsegnum in range(xsegstart,xsegmax):
	for ysegnum in range(ysegstart,ysegmax):
		#print("--------------",ysegnum,"  ",xsegnum)
		file = file_format + str(xsegnum) + file_format +str(ysegnum)
		seg_gcode_file = open((str(file) + '.gcode'),'w')
		print("Segmentaion Gcode file: ",seg_gcode_file)
		
		sx1 = xsegnum * seglen
		sy1 = ysegnum * segwidth
		sx2 = (xsegnum + 1) * seglen
		sy2 = (ysegnum + 1) * segwidth
		#print("x:" ,'(',sx1,',',sy1,')',"    Y:" ,'(',sx2,',',sy2,')')
		#print("segment number: " , "(",xsegnum , ",", ysegnum,")")
		
		for line in range(1,LINECOUNT):
			############# X and Y start and end values ############
			lx1 = XSTART[line]#3
			ly1 = YSTART[line]#50
			lx2 = XEND[line]#70
			ly2 = YEND[line]#50
			#print("XSATRT :", XSTART[line],",YSTART: ",YSTART[line],"                    XEND:",XEND[line],",YEND:",YEND[line])
			if lx1 == lx2: #Line is vertical
				#print("Line is vertical")
				if lx1 >= sx1 and lx1 <= sx2: 
					#print("Line is in Segment range")
					if ly1 > ly2:
						temp = ly1
						ly1 = ly2
						ly2 = temp

					#if ly1 >= sy1 and ly2 >= sy2 and ly1-sy1 == 0 or ly1-sy1 <= seglen and sy2-ly2 <= seglen:
					if ((ly1 >= sy1 and ly1 <= sy2) and (ly2 >= sy1 and ly2 <= sy2)): #verticla line is within the segment or just covering the entire segment
						print("vertical axis condition 1")
						xstart = lx1-sx1
						ystart = ly1 - sy1
						xend = lx2-sx1
						yend = ly2 - sy1
					
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertica line is spaning below and above the segment
						xstart = lx1 - sx1
						ystart = 0
						xend = lx2 - sx1
						yend = sy2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 >= sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertical line is start in segment or on segment line and end with out of segment in top side
						xstart = lx1 - sx1
						ystart = ly1 - sy1 
						xend  = lx2 - sx1
						yend = sy2 - sy1 
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 <= sy2: #Vertical line is start in segment or segment line and ens with out of segment in bottom side
						xstart =  lx1 - sx1
						ystart = 0
						xend = lx2 - sx1
						yend  = ly2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"

					
				else:
					print("Line may be next segment object")
					
			if ly1 == ly2: #Line is Horizantal
				#print("Line is horizontal")	
				if ly1 >= sy1 and ly1 <= sy2 :
					#print("Line is in Segment range")
					if lx1 > lx2:
						temp = lx1
						lx1 = lx2
						lx2 = temp
					#if ((lx2 - sx1 <= seglen) and (sx2 - lx1 <= seglen)) or ((sx1 <= lx2 and lx2 >= sx2) or (sx2-lx2 <= seglen)): #line is with in segment dimensions
							
					#if lX1 < sx1:
						#Xstart = sx1

					if lx1 >= sx1 and lx1 <=sx2 and lx2 >= sx1 and lx2 <= sx2: #Horizontal line is within the segment or just covering the entire segment
						xstart = lx1 - sx1
						ystart = ly1 - sy1
						xend = lx2 - sx1
						yend = ly2 - sy1
					
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: #horizantal line is spaning life and right of the segment
						xstart = 0
						ystart = ly1 - sy1
						xend = sx2 - sx1
						yend = ly2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 >= sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: # Horizontal line is start in segment or segment line and end with out of segment in right side
						xstart = lx1 - sx1
						ystart = ly1 - sy1 
						xend  = sx2 - sx1
						yend = ly2 - sy1 
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 <= sx2:#Horizantal line is start in segment or segment line and end with out of segment in lift side
						xstart =  0
						ystart = ly1 - sy1
						xend = lx2 - sx1
						yend  = ly2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
				else:
					print("Line may be next segment object")
			
		print(Gcode)
		seg_gcode_file.write(Gcode)
		seg_gcode_file.close()
		Gcode = ""
		gcodefilecount = gcodefilecount +1
		#time.sleep(1)
print(gcodefilecount , "Gcode file are successfully created.")

'''
import os
import sys

#openfile = sys.argv[1]
#savefile = sys.argv[2]

########## Segmentation Dimensions ###############
seglen = 18
segwidth = 12
layoutxmax = 108 #sys.argv[3]
layoutymax = 72 #sys.argv[4]
xsegnum = 0
ysegnum = 0
file_format = '00'
xsegstart = 0
ysegstart = 0
xsegmax = layoutxmax //seglen
ysegmax = layoutymax // segwidth
#print(xsegmax,ysegmax)
xstart = 0
xend =0
ystart =0
yend = 0

########## gcode variable ##################
Gcode = ""
gcodefilecount = 0
############# X and Y start and end values ############


for xsegnum in range(xsegstart,xsegmax):
	for ysegnum in range(ysegstart,ysegmax):		
		sx1 = xsegnum * seglen
		sy1 = ysegnum * segwidth
		sx2 = (xsegnum + 1) * seglen
		sy2 = (ysegnum + 1) * segwidth
		#print("x:" ,'(',sx1,',',sy1,')',"    Y:" ,'(',sx2,',',sy2,')')
		#print("segment number: " , "(",xsegnum , ",", ysegnum,")")

		file = file_format + str(xsegnum) + file_format +str(ysegnum)
		seg_gcode_file = open((str(file) + '.gcode'),'w')
		print("Segmentaion Gcode file: ",seg_gcode_file)
		for line in range(1,3):
			if line ==1:
				lx1 = 30
				ly1 = 55
				lx2 = 30
				ly2 = 70
			if line == 2:
				lx1 = 3
				ly1 = 50
				lx2 = 70
				ly2 = 50
			if lx1 == lx2: #Line is vertical
				print("Line is vertical")
				if lx1 >= sx1 and lx1 <= sx2: 
					print("Line is in Segment range")
					if ly1 > ly2:
						temp = ly1
						ly1 = ly2
						ly2 = temp

					#if ly1 >= sy1 and ly2 >= sy2 and ly1-sy1 == 0 or ly1-sy1 <= seglen and sy2-ly2 <= seglen:
					if ((ly1 >= sy1 and ly1 <= sy2) and (ly2 >= sy1 and ly2 <= sy2)): #verticla line is within the segment or just covering the entire segment
						print("vertical axis condition 1")
						xstart = lx1-sx1
						ystart = ly1 - sy1
						xend = lx2-sx1
						yend = ly2 - sy1
					
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertica line is spaning below and above the segment
						xstart = lx1 - sx1
						ystart = 0
						xend = lx2 - sx1
						yend = sy2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 >= sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2:
						xstart = lx1 - sx1
						ystart = ly1 - sy1 
						xend  = lx2 - sx1
						yend = sy2 - sy1 
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 <= sy2:
						xstart =  lx1 - sx1
						ystart = 0
						xend = lx2 - sx1
						yend  = ly2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"

					
				else:
					print("Line may be next segment object")
					
			if ly1 == ly2: #Line is Horizantal
				print("Line is horizontal")	
				if ly1 >= sy1 and ly1 <= sy2 :
					print("Line is in Segment range")
					if lx1 > lx2:
						temp = lx1
						lx1 = lx2
						lx2 = temp
					#if ((lx2 - sx1 <= seglen) and (sx2 - lx1 <= seglen)) or ((sx1 <= lx2 and lx2 >= sx2) or (sx2-lx2 <= seglen)): #line is with in segment dimensions
							
					#if lX1 < sx1:
						#Xstart = sx1

					if lx1 >= sx1 and lx1 <=sx2 and lx2 >= sx1 and lx2 <= sx2:
						xstart = lx1 - sx1
						ystart = ly1 - sy1
						xend = lx2 - sx1
						yend = ly2 - sy1
					
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: #horizantal line is spaning life and right of the segment
						xstart = 0
						ystart = ly1 - sy1
						xend = sx2 - sx1
						yend = ly2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 >= sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2:
						xstart = lx1 - sx1
						ystart = ly1 - sy1 
						xend  = sx2 - sx1
						yend = ly2 - sy1 
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 <= sx2:
						xstart =  0
						ystart = ly1 - sy1
						xend = lx2 - sx1
						yend  = ly2 - sy1
						Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
				else:
					print("Line may be next segment object")
			
		print(Gcode)
		seg_gcode_file.write(Gcode)
		seg_gcode_file.close()
		Gcode = ""
		gcodefilecount = gcodefilecount +1
print(gcodefilecount , "Gcode file are successfully created.")

'''







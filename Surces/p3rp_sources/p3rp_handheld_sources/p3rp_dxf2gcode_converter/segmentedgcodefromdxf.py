import os
import sys
import math
openfile = sys.argv[1]

########## Segmentation Dimensions ###############
seglen = 18
segwidth = 12
layoutxmax = 108 #sys.argv[2]
layoutymax = 72 #sys.argv[3]
#xsegnum = 0
#ysegnum = 0
#file_format = '00'
xsegstart = 0
ysegstart = 0
xsegmax = layoutxmax //seglen
ysegmax = layoutymax // segwidth
#print(xsegmax,ysegmax)
PIby180 = 3.1459/180
CircleStartA = 0
CircleEndA = 361
XMAX = 0
XMIN = 0
YMAX = 0
YMIN = 0
filexylen = 3
checkinpolygcode = 0
avoidrepeatedaction = 0
########## gcode variable ##################
Gcode = ""
gcodefilecount = 0

#LINE Buffer and Varibles
QTY  = 10000000
XSTART = [0 for i in range(QTY)]
YSTART = [0 for i in range(QTY)]
XEND   = [0 for i in range(QTY)]
YEND   = [0 for i in range(QTY)]
LINECOUNT	= 1

#CIRCLE Buffer and varibles
XCIRCLEPOS  = [0 for i in range(QTY)]
YCIRCLEPOS  = [0 for i in range(QTY)]
ZCIRCLEPOS   = [0 for i in range(QTY)]
CIRCLERADIUS = [0 for i in range(QTY)]
CIRCLECOUNT	= 1
CCount = 1
circlestartx = [0 for i in range(QTY)]
circlestarty = [0 for i in range(QTY)]
circleendx = [0 for i in range(QTY)]
circleendy = [0 for i in range(QTY)]
#ARC Buffer and varibles
StartAngle   = [0 for i in range(QTY)]
EndAngle   = [0 for i in range(QTY)] 
ARC_CenterX   = [0 for i in range(QTY)]
ARC_CenterY   = [0 for i in range(QTY)]
ZARK   = [0 for i in range(QTY)]
ARKRADIUS = [0 for i in range(QTY)]
ARKCOUNT    = 1
ACount = 1
arcstartx = [0 for i in range(QTY)]
arcstarty = [0 for i in range(QTY)]
arcendx = [0 for i in range(QTY)]
arcendy = [0 for i in range(QTY)]
#POLYLINE Buffer and Varibles
XPOLYSTART = [0 for i in range(QTY)]
YPOLYSTART = [0 for i in range(QTY)]
XPOLYEND   = [0 for i in range(QTY)]
YPOLYEND   = [0 for i in range(QTY)]
POLYLINECOUNT = 1
PLCount =1
polystartx = [0 for i in range(QTY)]
polystarty = [0 for i in range(QTY)]
ployendx = [0 for i in range(QTY)]
polyendy = [0 for i in range(QTY)]

DXFPATH, DXFNAME = os.path.split(openfile)

#### Read DXF file ############################################

file = open(openfile,errors='ignore')
while True: #### Main DXF read loop ####
	
	TEXT = file.readline()
	TEXT = TEXT.strip() # Remove spaces 
	#print(TEXT)
	'''if(TEXT == '$INSUNITS'):
		while True:			
			TEXT = file.readline()
			TEXT = TEXT.strip()
			print (TEXT)
			TEXT = file.readline()
			TEXT = TEXT.strip()
			print("dimension type:",TEXT)
			
					
			break
		break'''
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
	############## CIRCLE and ARC #####################
	if (TEXT == "AcDbCircle"): # Found circle
		CIRCLECOUNT = 1
		conditionbreakflag = 0
		while True:
			print(TEXT)
			TEXT = file.readline() # Read identifier
			TEXT = TEXT.strip()
			#print(">>>>>>>>>>>>CIRCLE>>>>>>>>>>>>>>>>>",TEXT)
			
			if (TEXT == "10"): 
				TEXT = file.readline() # Read X centre poosition
				TEXT = TEXT.strip()
				#print(">>>>>>>>>>>>CIRCLE TEXT of X-AXIS>>>>>>>>>>>>>>>>>",TEXT)
				XCIRCLEPOS[CIRCLECOUNT]= float(TEXT)
				TEXT = "NOTHING"
			
			if (TEXT == "20"): 
				TEXT = file.readline() # Read Y centre poosition
				TEXT = TEXT.strip()
				#print(">>>>>>>>>>>>>>CIRCLE TEXT of Y-AXIS>>>>>>>>>>>>>>>",TEXT)
				YCIRCLEPOS[CIRCLECOUNT]= float(TEXT)
				TEXT = "NOTHING"
			
			if (TEXT == "30"):
				TEXT = file.readline() # Read Z centre poosition
				TEXT = TEXT.strip()
				#print(">>>>>>>>>>>>>>CIRCLE TEXT of Z-AXIS>>>>>>>>>>>>>>>",TEXT)
				ZCIRCLEPOS[CIRCLECOUNT] = float(TEXT)
				TEXT = "NOTHING"							
			
			if (TEXT == "40"): 
				TEXT = file.readline() # Read Circele Radius
				TEXT = TEXT.strip()
				#print(">>>>>>>>>>>>>>CIRCLE RADIUS >>>>>>>>>>>>>>>",TEXT)
				CIRCLERADIUS[CIRCLECOUNT]= float(TEXT)
				#if CIRCLERADIUS[CIRCLECOUNT] > CIRCLERAD : BIGCOUNT = BIGCOUNT + 1
				TEXT = "NOTHING"
			if(TEXT == "210"):
				print("================210=================")
			if(TEXT == "220"):
				print("================220==================")
			if(TEXT == "230"):
				print("================230===================")
			if( TEXT == "$ANGDIR"):
				print("===============ANGDIR================")
				
			##################   ARC ##################################	
			if (TEXT == "AcDbArc"):  #Found ARC	
				ARKCOUNT = 1
				while True:			
					TEXT = file.readline() # Read identifier
					TEXT = TEXT.strip()
					print("|||||||||||||||ARC |||||||||||||||",TEXT,ARKCOUNT)
					if (TEXT == "50"): 
						TEXT = file.readline() # Read Start Angle
						TEXT = TEXT.strip()
						#print("||||||||||||||ARC Starting angle ||||||||||||||||",TEXT,ARKCOUNT)
						StartAngle[ARKCOUNT]= float(TEXT)
						TEXT = "NOTHING"
				
					if (TEXT == "51"): 
						TEXT = file.readline() # Read End Angle
						TEXT = TEXT.strip()
						#print("||||||||||||||ARC Ending angle ||||||||||||||||",TEXT,ARKCOUNT)
						EndAngle[ARKCOUNT]= float(TEXT)
						TEXT = "NOTHING"
						
					if (TEXT == "0"): # No more data
						if StartAngle[ARKCOUNT] > XMAX : XMAX = StartAngle[ARKCOUNT]
						if StartAngle[ARKCOUNT] < XMIN : XMIN = StartAngle[ARKCOUNT]
						if EndAngle[ARKCOUNT] > YMAX : YMAX = EndAngle[ARKCOUNT]
						if EndAngle[ARKCOUNT] < YMIN : YMIN = EndAngle[ARKCOUNT]
						ARC_CenterX[ARKCOUNT] = XCIRCLEPOS[CIRCLECOUNT] #copy the starting X position
						ARC_CenterY[ARKCOUNT] = YCIRCLEPOS[CIRCLECOUNT] #Copy the Sarting Y position
						ZARK[ARKCOUNT] = ZCIRCLEPOS[CIRCLECOUNT] #Copy the Starting Z postion
						ARKRADIUS[ARKCOUNT] = CIRCLERADIUS[CIRCLECOUNT] #Copy the Radius 
						ARKCOUNT = ARKCOUNT + 1												
						TEXT = file.readline()
						TEXT = TEXT.strip()
						#print("----------ARC TEXT of 0-------------",TEXT)
						if(TEXT == "CIRCLE"):						
							TEXT = "NOTHING"
						#print("-----------------------",TEXT);						
						conditionbreakflag = 1						
						break
				if(conditionbreakflag): 
					CIRCLECOUNT = 1
					break				

			if (TEXT == "0"): # No more data
				if XCIRCLEPOS[CIRCLECOUNT] > XMAX : XMAX = XCIRCLEPOS[CIRCLECOUNT]
				if XCIRCLEPOS[CIRCLECOUNT] < XMIN : XMIN = XCIRCLEPOS[CIRCLECOUNT]
				if YCIRCLEPOS[CIRCLECOUNT] > YMAX : YMAX = YCIRCLEPOS[CIRCLECOUNT]
				if YCIRCLEPOS[CIRCLECOUNT] < YMIN : YMIN = YCIRCLEPOS[CIRCLECOUNT]
				CIRCLECOUNT = CIRCLECOUNT + 1
				break
		###################### Create G-Code for Circle ###############################
		for i in range(1,CIRCLECOUNT):
			ARKCOUNT = ARKCOUNT -1
			print ("Circle X centre", XCIRCLEPOS[i], " Y centre", YCIRCLEPOS[i] , " Radius" , CIRCLERADIUS[i])

			xpresentpos = round(XCIRCLEPOS[i]-CIRCLERADIUS[i])
			ypresentpos = round(YCIRCLEPOS[i])
			for theta in range(CircleStartA,CircleEndA):
				circlestartx[theta] = xpresentpos
				circlestarty[theta] = ypresentpos
				#Apply Parametric equation for the circle
				circleendx[theta] = XCIRCLEPOS[i]+CIRCLERADIUS[i]*math.cos(theta*PIby180)
				circleendy[theta] = YCIRCLEPOS[i]+CIRCLERADIUS[i]*math.sin(theta*PIby180)
				xpresentpos = circleendx[theta]
				ypresentpos = circleendy[theta]
				CCount = CCount + 1					
			
		##################### Split ARK to lines using Paramtric equation #########################
		for i in range(1,ARKCOUNT):			
			print ("ARC Start angle X centre", StartAngle[i], " End angle Y centre", EndAngle[i] , " Radius" , ARKRADIUS[i])
			print ("ARC X centre", ARC_CenterX[i], " Y centre", ARC_CenterY[i] , " Radius" , ARKRADIUS[i])			
			R = ARKRADIUS[i]
			SA = int(StartAngle[i]) # SA = ARC Satarting Angle
			EA = int(EndAngle[i])	# EA = ARC Ending Angle
			xpresentpos = ARC_CenterX[i]
			ypresentpos = ARC_CenterY[i]
			if(SA < EA):
				for j in range(SA,EA):
					arcstartx[ACount] = xpresentpos
					arcstarty[ACount] = ypresentpos					
					#Apply Parametric equation for the circle
					arcendx[ACount] = ARC_CenterX[i]+R*math.cos(j*PIby180)
					arcendy[ACount] = ARC_CenterY[i]+R*math.sin(j*PIby180)
					xpresentpos = arcendx[ACount]
					ypresentpos = arcendy[ACount]
					ACount = ACount + 1
			if(SA > EA):
				for j in range(SA ,(360-EA)):
					arcstartx[ACount] = xpresentpos
					arcstarty[ACount] = ypresentpos
					#Apply Parametric equation for the circle
					arcendx[ACount] = ARC_CenterX[i]+R*math.sin(j*PIby180)
					arcendy[ACount] = ARC_CenterY[i]+R*math.cos(j*PIby180)
					xpresentpos = arcendx[ACount]
					ypresentpos = arcendy[ACount]
					ACount = ACount + 1
					
	############## POLYLINE #####################		
	if (TEXT == "AcDbPolyline"): # Found polyline
		POLYLINECOUNT = 1
		while True:
			#print(TEXT)
			TEXT = file.readline() # Read identifier
			TEXT = TEXT.strip()
			#print("++++++++++++++++++" ,TEXT)

			if (TEXT == "10"): 
				TEXT = file.readline() # Read X start poosition (Vertex X coordinates)
				TEXT = TEXT.strip()
				print(TEXT)
				XPOLYSTART[POLYLINECOUNT]= float(TEXT)
				print("..................",XPOLYSTART[POLYLINECOUNT],".......POLYLINE X-AXIS.....................",POLYLINECOUNT)
				TEXT="NOTHING"
				checkinpolygcode = checkinpolygcode+1
			
			if (TEXT == "20"): 
				TEXT = file.readline() # Read Y start poosition ( Y value of vertex coordinates)
				TEXT = TEXT.strip()
				print(TEXT)
				YPOLYSTART[POLYLINECOUNT-1]= float(TEXT)
				print("..................",YPOLYSTART[POLYLINECOUNT-1],".........POLYLINE Y-AXIS............",POLYLINECOUNT-1)
				TEXT = "NOTHING"
				checkinpolygcode = checkinpolygcode+1
				
			if (TEXT == "90"):
				TEXT = file.readline() # Number of vertices
				TEXT = TEXT.strip()
				print("**********POLYLINE vertices**************" ,TEXT)
				TEXT = file.readline()
				TEXT = TEXT.strip()
				if(TEXT == "70"):
					TEXT = file.readline() #Polyline flag
					TEXT = TEXT.strip()
					print("********** POLYLINE 70**************" ,TEXT)
					if(TEXT == "0" or TEXT == "1"):
						TEXT = "1"
						TEXT = file.readline()
						TEXT = TEXT.strip()
						print("************************" ,TEXT)
						TEXT = file.readline()
						TEXT = TEXT.strip()	
						print("************************" ,TEXT)
						
			if (TEXT == "0"): # No more data
				break
				
			if (YPOLYSTART[POLYLINECOUNT] > YMAX): YMAX = YPOLYSTART[POLYLINECOUNT]
			if (YPOLYSTART[POLYLINECOUNT] < YMIN): YMIN = YPOLYSTART[POLYLINECOUNT]
			if (XPOLYSTART[POLYLINECOUNT] > XMAX): XMAX = XPOLYSTART[POLYLINECOUNT]
			if (XPOLYSTART[POLYLINECOUNT] < XMIN): XMIN = XPOLYSTART[POLYLINECOUNT]	
			POLYLINECOUNT = POLYLINECOUNT + 1
			
			######### Creating GCODE for PolyLines #################################
			
			while(checkinpolygcode == 2):
				checkinpolygcode = 0				
				print("________________________________________________>",TEXT)

				for i in range(1,POLYLINECOUNT):
					#print ("Line X start" , XPOLYSTART[i] , " Y start" , YPOLYSTART[i])
					
					#xpresentpos = round(XPOLYSTART[i])
					#ypresentpos = round(YPOLYSTART[i])
					if((avoidrepeatedaction < i) and (XPOLYSTART[i] > 0) and (YPOLYSTART[i]) > 0):											
						polystartx[i] = xpresentpos
						polystarty[i] = ypresentpos
						ployendx[i] = round(XPOLYSTART[i])
						polyendy[i] = round(YPOLYSTART[i])
						
						xpresentpos = ployendx[i]
						ypresentpos = polyendy[i]							
				avoidrepeatedaction=i	
	if( TEXT == "EOF"):
		ARKCOUNT = ARKCOUNT + 1
		break
file.close()
print("file reading is completed")
print("Line count: " , LINECOUNT-1)
print("Arc count: ",ARKCOUNT-1 ,ACount)
print("Circle count: ",CIRCLECOUNT-1,CCount)
print("Polyline count: ",POLYLINECOUNT-1)
#for i in range (1,ACount):
#	print("XSATRT :", arcstartx[i],",YSTART: ",arcstarty[i],"                    XEND:",arcendx[i],",YEND:",arcendy[i])

########################## Gcode using segmentation ###########################
for xsegnum in range(xsegstart,xsegmax):
	for ysegnum in range(ysegstart,ysegmax):
		#print("--------------",ysegnum,"  ",xsegnum)
		
		############## file name format #####################
		xsnum = str(xsegnum)
		ysnum = str(ysegnum)
		xlen = filexylen - len(xsnum)
		ylen = filexylen - len(ysnum)
		#print(xlen,ylen)
		for i in range(0,xlen):
			xsnum = '0'+ str(xsnum)
		for j in range(0,ylen):
			ysnum = '0' + str(ysnum)
		#print(xsnum,ysnum)		
		file = str(xsnum) + str(ysnum) 
		#file = file_format + str(xsegnum) + file_format +str(ysegnum)
		seg_gcode_file = open((str(file) + '.gcode'),'w')   #file created like xxxyyy.gcode
		print("Segmentaion Gcode file: ",seg_gcode_file)
		xlen = 0
		ylen = 0
		
		################## Segment dimensions creation #######################
		sx1 = xsegnum * seglen
		sy1 = ysegnum * segwidth
		sx2 = (xsegnum + 1) * seglen
		sy2 = (ysegnum + 1) * segwidth
		#print("x:" ,'(',sx1,',',sy1,')',"    Y:" ,'(',sx2,',',sy2,')')
		#print("segment number: " , "(",xsegnum , ",", ysegnum,")")
		
		########################GCODE for LINE ##################################
		for line in range(1,LINECOUNT):
			############# X and Y start and end values ############
			lx1 = round(XSTART[line])#3
			ly1 = round(YSTART[line])#50
			lx2 = round(XEND[line])#70
			ly2 = round(YEND[line])#50
			#print("XSATRT :", XSTART[line],",YSTART: ",YSTART[line],"                    XEND:",XEND[line],",YEND:",YEND[line])
			
			####################### Vertical Segment ############################
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
						#print("vertical axis condition 1")
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
			
			########################### Horizantal segment ######################		
			if ly1 == ly2: #Line is Horizantal
				#print("Line is horizontal")	
				if ly1 >= sy1 and ly1 <= sy2 :
					#print("Line is in Segment range")
					if lx1 > lx2:
						temp = lx1
						lx1 = lx2
						lx2 = temp
					#if ((lx2 - sx1 <= seglen) and (sx2 - lx1 <= seglen)) or ((sx1 <= lx2 and lx2 >= sx2) or (sx2-lx2 <= seglen)): #line is with in segment dimensions

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
					
		#################### GCODE for ARC #####################################
		for ac in range(1,ARKCOUNT):
			for line in range(1,ACount):
				############# X and Y start and end values ############
				lx1 = round(arcstartx[line])#3
				ly1 = round(arcstarty[line])#50
				lx2 = round(arcendx[line])#70
				ly2 = round(arcendy[line])#50
				#print("arcstartx:",arcstartx[line],",",lx1,"                  arcstarty:",arcstarty[line],",",ly1)
				#print("arcendx:",arcendx[line],",",lx2,"                   arcendy:",arcendy[line],",",ly2)
				
				################# Vertical Segment #############################
				if lx1 == lx2: #arc Line is vertical
					#print("Line is vertical")
					if lx1 >= sx1 and lx1 <= sx2: 
						#print("Line is in Segment range")
						if ly1 > ly2:
							temp = ly1
							ly1 = ly2
							ly2 = temp

						#if ly1 >= sy1 and ly2 >= sy2 and ly1-sy1 == 0 or ly1-sy1 <= seglen and sy2-ly2 <= seglen:
						if ((ly1 >= sy1 and ly1 <= sy2) and (ly2 >= sy1 and ly2 <= sy2)): #verticla line is within the segment or just covering the entire segment
							#print("vertical axis condition 1")
							xstart = lx1-sx1
							ystart = ly1 - sy1
							xend = lx2-sx1
							yend = ly2 - sy1
						
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertica line is spaning below and above the segment
							#print("vertical axis condition 2")
							xstart = lx1 - sx1
							ystart = 0
							xend = lx2 - sx1
							yend = sy2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif ly1 >= sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertical line is start in segment or on segment line and end with out of segment in top side
							#print("vertical axis condition 3")
							xstart = lx1 - sx1
							ystart = ly1 - sy1 
							xend  = lx2 - sx1
							yend = sy2 - sy1 
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 <= sy2: #Vertical line is start in segment or segment line and ens with out of segment in bottom side
							#print("vertical axis condition 4")
							xstart =  lx1 - sx1
							ystart = 0
							xend = lx2 - sx1
							yend  = ly2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"

					else:
						print("Line may be next segment object")
				
				######################### Horizantal Segment ##############################	
				if ly1 == ly2: #arc Line is Horizantal
					#print("Line is horizontal")	
					if ly1 >= sy1 and ly1 <= sy2 :
						#print("Line is in Segment range")
						if lx1 > lx2:
							temp = lx1
							lx1 = lx2
							lx2 = temp
						#if ((lx2 - sx1 <= seglen) and (sx2 - lx1 <= seglen)) or ((sx1 <= lx2 and lx2 >= sx2) or (sx2-lx2 <= seglen)): #line is with in segment dimensions

						if lx1 >= sx1 and lx1 <=sx2 and lx2 >= sx1 and lx2 <= sx2: #Horizontal line is within the segment or just covering the entire segment
							#print("HC 1")
							xstart = lx1 - sx1
							ystart = ly1 - sy1
							xend = lx2 - sx1
							yend = ly2 - sy1
						
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: #horizantal line is spaning life and right of the segment
							#print("HC 2")
							xstart = 0
							ystart = ly1 - sy1
							xend = sx2 - sx1
							yend = ly2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif lx1 >= sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: # Horizontal line is start in segment or segment line and end with out of segment in right side
							#print("HC 3")
							xstart = lx1 - sx1
							ystart = ly1 - sy1 
							xend  = sx2 - sx1
							yend = ly2 - sy1 
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 <= sx2:#Horizantal line is start in segment or segment line and end with out of segment in lift side
							#print("HC 4")
							xstart =  0
							ystart = ly1 - sy1
							xend = lx2 - sx1
							yend  = ly2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					else:
						print("Line may be next segment object")
						
		################### GCODE for CIRCLE ################################
		for cc in range(1,CIRCLECOUNT):
			for line in range(1,CCount):
				############# X and Y start and end values ############
				lx1 = round(circlestartx[line])#3
				ly1 = round(circlestarty[line])#50
				lx2 = round(circleendx[line])#70
				ly2 = round(circleendy[line])#50
				#print("arcstartx:",arcstartx[line],",",lx1,"                  arcstarty:",arcstarty[line],",",ly1)
				#print("arcendx:",arcendx[line],",",lx2,"                   arcendy:",arcendy[line],",",ly2)
				
				#################### Vertical Segment ##########################
				if lx1 == lx2: #circle Line is vertical
					#print("Line is vertical")
					if lx1 >= sx1 and lx1 <= sx2: 
						#print("Line is in Segment range")
						if ly1 > ly2:
							temp = ly1
							ly1 = ly2
							ly2 = temp

						#if ly1 >= sy1 and ly2 >= sy2 and ly1-sy1 == 0 or ly1-sy1 <= seglen and sy2-ly2 <= seglen:
						if ((ly1 >= sy1 and ly1 <= sy2) and (ly2 >= sy1 and ly2 <= sy2)): #verticla line is within the segment or just covering the entire segment
							#print("vertical axis condition 1")
							xstart = lx1-sx1
							ystart = ly1 - sy1
							xend = lx2-sx1
							yend = ly2 - sy1
						
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertica line is spaning below and above the segment
							#print("vertical axis condition 2")
							xstart = lx1 - sx1
							ystart = 0
							xend = lx2 - sx1
							yend = sy2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif ly1 >= sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertical line is start in segment or on segment line and end with out of segment in top side
							#print("vertical axis condition 3")
							xstart = lx1 - sx1
							ystart = ly1 - sy1 
							xend  = lx2 - sx1
							yend = sy2 - sy1 
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 <= sy2: #Vertical line is start in segment or segment line and ens with out of segment in bottom side
							#print("vertical axis condition 4")
							xstart =  lx1 - sx1
							ystart = 0
							xend = lx2 - sx1
							yend  = ly2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"

					else:
						print("Line may be next segment object")
				
				###################### Horizantal Segment #############################
				if ly1 == ly2: #circle Line is Horizantal
					#print("Line is horizontal")	
					if ly1 >= sy1 and ly1 <= sy2 :
						#print("Line is in Segment range")
						if lx1 > lx2:
							temp = lx1
							lx1 = lx2
							lx2 = temp
						#if ((lx2 - sx1 <= seglen) and (sx2 - lx1 <= seglen)) or ((sx1 <= lx2 and lx2 >= sx2) or (sx2-lx2 <= seglen)): #line is with in segment dimensions

						if lx1 >= sx1 and lx1 <=sx2 and lx2 >= sx1 and lx2 <= sx2: #Horizontal line is within the segment or just covering the entire segment
							#print("HC 1")
							xstart = lx1 - sx1
							ystart = ly1 - sy1
							xend = lx2 - sx1
							yend = ly2 - sy1
						
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: #horizantal line is spaning life and right of the segment
							#print("HC 2")
							xstart = 0
							ystart = ly1 - sy1
							xend = sx2 - sx1
							yend = ly2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif lx1 >= sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: # Horizontal line is start in segment or segment line and end with out of segment in right side
							#print("HC 3")
							xstart = lx1 - sx1
							ystart = ly1 - sy1 
							xend  = sx2 - sx1
							yend = ly2 - sy1 
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
						elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 <= sx2:#Horizantal line is start in segment or segment line and end with out of segment in lift side
							#print("HC 4")
							xstart =  0
							ystart = ly1 - sy1
							xend = lx2 - sx1
							yend  = ly2 - sy1
							Gcode = Gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
							Gcode = Gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					else:
						print("Line may be next segment object")
						
		########################GCODE for POLYLINE ##################################
		for pline in range(1,POLYLINECOUNT):
			############# X and Y start and end values ############
			lx1 = round(polystartx[pline])#3
			ly1 = round(polystarty[pline])#50
			lx2 = round(ployendx[pline])#70
			ly2 = round(polyendy[pline])#50
			#print("XSATRT :", XSTART[line],",YSTART: ",YSTART[line],"                    XEND:",XEND[line],",YEND:",YEND[line])
			
			####################### Vertical Segment ############################
			if lx1 == lx2: #PolyLine is vertical
				#print("Line is vertical")
				if lx1 >= sx1 and lx1 <= sx2: 
					#print("Line is in Segment range")
					if ly1 > ly2:
						temp = ly1
						ly1 = ly2
						ly2 = temp

					#if ly1 >= sy1 and ly2 >= sy2 and ly1-sy1 == 0 or ly1-sy1 <= seglen and sy2-ly2 <= seglen:
					if ((ly1 >= sy1 and ly1 <= sy2) and (ly2 >= sy1 and ly2 <= sy2)): #verticla line is within the segment or just covering the entire segment
						#print("vertical axis condition 1")
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
			########################### Horizantal segment ######################		
			if ly1 == ly2: #PolyLine is Horizantal
				#print("Line is horizontal")	
				if ly1 >= sy1 and ly1 <= sy2 :
					#print("Line is in Segment range")
					if lx1 > lx2:
						temp = lx1
						lx1 = lx2
						lx2 = temp

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







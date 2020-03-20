import os
import sys
import math
openfile = sys.argv[1]

#Segmentation Dimensions and variable
seglen = 18
segwidth = 12
#layoutxmax = 108 #sys.argv[2]
#layoutymax = 72 #sys.argv[3]
#xsegnum = 0
#ysegnum = 0
#file_format = '00'
xsegstart = 0
ysegstart = 0
#xsegmax = layoutxmax //seglen
#ysegmax = layoutymax // segwidth
#print(xsegmax,ysegmax)
PIby180 = 3.1459/180
quantity  = 10000000
xmax = 0
xmin = 0
ymax = 0
ymin = 0
xylenforfile = 3

#Layout maximum values buffers
layoutxmax = [0 for i in range(1)]
layoutymax = [0 for i in range(1)]
layoutzmax = [0 for i in range(1)]

#Gcode variable 
gcode = ""
gcodefilescount = 0

#LINE Buffer and Varibles
xstart = [0 for i in range(quantity)]
ystart = [0 for i in range(quantity)]
xend   = [0 for i in range(quantity)]
yend   = [0 for i in range(quantity)]
linecount	= 1

#CIRCLE Buffer and varibles
circlexpos  = [0 for i in range(quantity)]
circleypos  = [0 for i in range(quantity)]
circlezpos   = [0 for i in range(quantity)]
circleradius = [0 for i in range(quantity)]
circlecount	= 1
CCount = 1
circlestartx = [0 for i in range(quantity)]
circlestarty = [0 for i in range(quantity)]
circleendx = [0 for i in range(quantity)]
circleendy = [0 for i in range(quantity)]
circlestartangle = 0
circleendangle = 361

#ARC Buffer and varibles
startangle   = [0 for i in range(quantity)]
endangle   = [0 for i in range(quantity)] 
arcxcenter   = [0 for i in range(quantity)]
arcycenter   = [0 for i in range(quantity)]
arczcenter   = [0 for i in range(quantity)]
arcradius = [0 for i in range(quantity)]
arccount    = 1
ACount = 1
arcstartx = [0 for i in range(quantity)]
arcstarty = [0 for i in range(quantity)]
arcendx = [0 for i in range(quantity)]
arcendy = [0 for i in range(quantity)]

#POLYLINE Buffer and Varibles
xpolystart = [0 for i in range(quantity)]
ypolystart = [0 for i in range(quantity)]
polylinecount = 1
PLCount =1
polystartx = [0 for i in range(quantity)]
polystarty = [0 for i in range(quantity)]
polyendx = [0 for i in range(quantity)]
polyendy = [0 for i in range(quantity)]
xpolypresentpos = 0
ypolypresentpos = 0
checkinpolygcode = 0
avoidrepeatedaction = 0

DXFPATH, DXFNAME = os.path.split(openfile)

#### Open and Read DXF file ############################################
file = open(openfile,errors='ignore')
while True: #### Main DXF read loop ####	
	filereadline = file.readline()
	filereadline = filereadline.strip() # Remove spaces 
	#print(filereadline)
	
	####################### Read x,y and z maximum value #########################
	if(filereadline == "$EXTMAX"):
		while True:
			filereadline = file.readline()
			filereadline = filereadline.strip() 
			print("$EXTMAX",filereadline)
			if(filereadline == "10"): 
				filereadline = file.readline()
				filereadline = filereadline.strip()
				layoutxmax = float(filereadline) #X maximum value
				filereadline = "NOTHING"
			if(filereadline == "20"):
				filereadline = file.readline()
				filereadline = filereadline.strip()
				layoutymax = float(filereadline) #Y maximum value
				filereadline = "NOTHING"
			if(filereadline == "30"): 
				filereadline = file.readline()
				filereadline = filereadline.strip()
				layoutzmax = float(filereadline) #Z maximum value
				filereadline = "NOTHING"
			if(filereadline == "9"):
				break
				
	'''if(filereadline == '$INSUNITS'):
		while True:			
			filereadline = file.readline()
			filereadline = filereadline.strip()
			print (filereadline)
			filereadline = file.readline()
			filereadline = filereadline.strip()
			print("dimension type:",filereadline)
			
					
			break
		break'''
	############## LINE #####################
	if (filereadline == "AcDbLine"): # Found line
		#linecount = 1
		while True:
			filereadline = file.readline() # Read identifier
			filereadline = filereadline.strip()
			#print(filereadline)
			if (filereadline == "10"): 
				filereadline = file.readline() # Read X start poosition
				filereadline = filereadline.strip()
				xstart[linecount]= float(filereadline)
				filereadline="NOTHING"
			
			if (filereadline == "20"): 
				filereadline = file.readline() # Read Y start poosition
				filereadline = filereadline.strip()
				ystart[linecount]= float(filereadline)
				filereadline = "NOTHING"

			if (filereadline == "11"): 
				filereadline = file.readline() # Read X end poosition
				filereadline = filereadline.strip()
				xend[linecount]= float(filereadline)
				filereadline = "NOTHING"
			
			if (filereadline == "21"): 
				filereadline = file.readline() # Read Y end poosition
				filereadline = filereadline.strip()
				#print("..........................LINE..........",filereadline)
				yend[linecount]= float(filereadline)
				filereadline = "NOTHING"
			
			if (filereadline == "0"): # No more data
				if (xstart[linecount] > xmax): xmax = xstart[linecount]
				if (ystart[linecount] < ymin): ymin = ystart[linecount]
				if (yend[linecount] > ymax): ymax = yend[linecount]
				if (xend[linecount] < xmin): xmin = xend[linecount]
				
				linecount = linecount + 1
				break
	############## CIRCLE and ARC #####################
	if (filereadline == "AcDbCircle"): # Found circle
		circlecount = 1
		conditionbreakflag = 0
		while True:
			print(filereadline)
			filereadline = file.readline() # Read identifier
			filereadline = filereadline.strip()
			#print(">>>>>>>>>>>>CIRCLE>>>>>>>>>>>>>>>>>",filereadline)
			
			if (filereadline == "10"): 
				filereadline = file.readline() # Read X centre poosition
				filereadline = filereadline.strip()
				#print(">>>>>>>>>>>>CIRCLE filereadline of X-AXIS>>>>>>>>>>>>>>>>>",filereadline)
				circlexpos[circlecount]= float(filereadline)
				filereadline = "NOTHING"
			
			if (filereadline == "20"): 
				filereadline = file.readline() # Read Y centre poosition
				filereadline = filereadline.strip()
				#print(">>>>>>>>>>>>>>CIRCLE filereadline of Y-AXIS>>>>>>>>>>>>>>>",filereadline)
				circleypos[circlecount]= float(filereadline)
				filereadline = "NOTHING"
			
			if (filereadline == "30"):
				filereadline = file.readline() # Read Z centre poosition
				filereadline = filereadline.strip()
				#print(">>>>>>>>>>>>>>CIRCLE filereadline of Z-AXIS>>>>>>>>>>>>>>>",filereadline)
				circlezpos[circlecount] = float(filereadline)
				filereadline = "NOTHING"							
			
			if (filereadline == "40"): 
				filereadline = file.readline() # Read Circele Radius
				filereadline = filereadline.strip()
				#print(">>>>>>>>>>>>>>CIRCLE RADIUS >>>>>>>>>>>>>>>",filereadline)
				circleradius[circlecount]= float(filereadline)
				#if circleradius[circlecount] > CIRCLERAD : BIGCOUNT = BIGCOUNT + 1
				filereadline = "NOTHING"
			if(filereadline == "210"):
				print("================210=================")
			if(filereadline == "220"):
				print("================220==================")
			if(filereadline == "230"):
				print("================230===================")
			if( filereadline == "$ANGDIR"):
				print("===============ANGDIR================")
				
			##################   ARC ##################################	
			if (filereadline == "AcDbArc"):  #Found ARC	
				arccount = 1
				while True:			
					filereadline = file.readline() # Read identifier
					filereadline = filereadline.strip()
					print("|||||||||||||||ARC |||||||||||||||",filereadline,arccount)
					if (filereadline == "50"): 
						filereadline = file.readline() # Read Start Angle
						filereadline = filereadline.strip()
						#print("||||||||||||||ARC Starting angle ||||||||||||||||",filereadline,arccount)
						startangle[arccount]= float(filereadline)
						filereadline = "NOTHING"
				
					if (filereadline == "51"): 
						filereadline = file.readline() # Read End Angle
						filereadline = filereadline.strip()
						#print("||||||||||||||ARC Ending angle ||||||||||||||||",filereadline,arccount)
						endangle[arccount]= float(filereadline)
						filereadline = "NOTHING"
						
					if (filereadline == "0"): # No more data
						if startangle[arccount] > xmax : xmax = startangle[arccount]
						if startangle[arccount] < xmin : xmin = startangle[arccount]
						if endangle[arccount] > ymax : ymax = endangle[arccount]
						if endangle[arccount] < ymin : ymin = endangle[arccount]
						arcxcenter[arccount] = circlexpos[circlecount] #copy the starting X position
						arcycenter[arccount] = circleypos[circlecount] #Copy the Sarting Y position
						arczcenter[arccount] = circlezpos[circlecount] #Copy the Starting Z postion
						arcradius[arccount] = circleradius[circlecount] #Copy the Radius 
						arccount = arccount + 1												
						filereadline = file.readline()
						filereadline = filereadline.strip()
						#print("----------ARC filereadline of 0-------------",filereadline)
						if(filereadline == "CIRCLE"):						
							filereadline = "NOTHING"
						#print("-----------------------",filereadline);						
						conditionbreakflag = 1						
						break
				if(conditionbreakflag): 
					circlecount = 1
					break				

			if (filereadline == "0"): # No more data
				if circlexpos[circlecount] > xmax : xmax = circlexpos[circlecount]
				if circlexpos[circlecount] < xmin : xmin = circlexpos[circlecount]
				if circleypos[circlecount] > ymax : ymax = circleypos[circlecount]
				if circleypos[circlecount] < ymin : ymin = circleypos[circlecount]
				circlecount = circlecount + 1
				break
		###################### Create G-Code for Circle ###############################
		for i in range(1,circlecount):
			arccount = arccount -1
			print ("Circle X centre", circlexpos[i], " Y centre", circleypos[i] , " Radius" , circleradius[i])

			xpresentpos = round(circlexpos[i]-circleradius[i])
			ypresentpos = round(circleypos[i])
			for theta in range(circlestartangle,circleendangle):
				circlestartx[theta] = xpresentpos
				circlestarty[theta] = ypresentpos
				#Apply Parametric equation for the circle
				circleendx[theta] = circlexpos[i]+circleradius[i]*math.cos(theta*PIby180)
				circleendy[theta] = circleypos[i]+circleradius[i]*math.sin(theta*PIby180)
				xpresentpos = circleendx[theta]
				ypresentpos = circleendy[theta]
				CCount = CCount + 1					
			
		##################### Split ARK to lines using Paramtric equation #########################
		for i in range(1,arccount):			
			print ("ARC Start angle X centre", startangle[i], " End angle Y centre", endangle[i] , " Radius" , arcradius[i])
			print ("ARC X centre", arcxcenter[i], " Y centre", arcycenter[i] , " Radius" , arcradius[i])			
			R = arcradius[i]
			SA = int(startangle[i]) # SA = ARC Satarting Angle
			EA = int(endangle[i])	# EA = ARC Ending Angle
			xpresentpos = arcxcenter[i]
			ypresentpos = arcycenter[i]
			if(SA < EA):
				for j in range(SA,EA):
					arcstartx[ACount] = xpresentpos
					arcstarty[ACount] = ypresentpos					
					#Apply Parametric equation for the circle
					arcendx[ACount] = arcxcenter[i]+R*math.cos(j*PIby180)
					arcendy[ACount] = arcycenter[i]+R*math.sin(j*PIby180)
					xpresentpos = arcendx[ACount]
					ypresentpos = arcendy[ACount]
					ACount = ACount + 1
			if(SA > EA):
				for j in range(SA ,(360-EA)):
					arcstartx[ACount] = xpresentpos
					arcstarty[ACount] = ypresentpos
					#Apply Parametric equation for the circle
					arcendx[ACount] = arcxcenter[i]+R*math.sin(j*PIby180)
					arcendy[ACount] = arcycenter[i]+R*math.cos(j*PIby180)
					xpresentpos = arcendx[ACount]
					ypresentpos = arcendy[ACount]
					ACount = ACount + 1
					
	############## POLYLINE #####################		
	if (filereadline == "AcDbPolyline"): # Found polyline
		polylinecount = 1
		avoidrepeatedaction = 1
		while True:
			#print(filereadline)
			filereadline = file.readline() # Read identifier
			filereadline = filereadline.strip()
			#print("++++++++++++++++++" ,filereadline)

			if (filereadline == "10"): 
				filereadline = file.readline() # Read X start poosition (Vertex X coordinates)
				filereadline = filereadline.strip()
				#print(filereadline)
				xpolystart[polylinecount]= float(filereadline)
				#print("..................",xpolystart[polylinecount],".......POLYLINE X-AXIS.....................",polylinecount)
				filereadline="NOTHING"
				checkinpolygcode = checkinpolygcode+1
			
			if (filereadline == "20"): 
				filereadline = file.readline() # Read Y start poosition ( Y value of vertex coordinates)
				filereadline = filereadline.strip()
				#print(filereadline)
				ypolystart[polylinecount-1]= float(filereadline)
				#print("..................",ypolystart[polylinecount-1],".........POLYLINE Y-AXIS............",polylinecount-1)
				filereadline = "NOTHING"
				checkinpolygcode = checkinpolygcode+1
				
			if (filereadline == "90"):
				filereadline = file.readline() # Number of vertices
				filereadline = filereadline.strip()
				#print("**********POLYLINE vertices**************" ,filereadline)
				filereadline = file.readline()
				filereadline = filereadline.strip()
				if(filereadline == "70"):
					filereadline = file.readline() #Polyline flag
					filereadline = filereadline.strip()
					#print("********** POLYLINE 70**************" ,filereadline)
					if(filereadline == "0" or filereadline == "1"):
						filereadline = "1"
						filereadline = file.readline()
						filereadline = filereadline.strip()
						#print("************************" ,filereadline)
						filereadline = file.readline()
						filereadline = filereadline.strip()	
						#print("************************" ,filereadline)
						
			if (filereadline == "0"): # No more data
				break
				
			if (ypolystart[polylinecount] > ymax): ymax = ypolystart[polylinecount]
			if (ypolystart[polylinecount] < ymin): ymin = ypolystart[polylinecount]
			if (xpolystart[polylinecount] > xmax): xmax = xpolystart[polylinecount]
			if (xpolystart[polylinecount] < xmin): xmin = xpolystart[polylinecount]	
			polylinecount = polylinecount + 1
			
			######### Creating GCODE for PolyLines #################################
			
			while(checkinpolygcode == 2):
				checkinpolygcode = 0				
				#print("________________________________________________>",filereadline)				
				for i in range(avoidrepeatedaction,polylinecount):
					#print (" poly Line X start" , xpolystart[i] , " Y start" , ypolystart[i])					
					#xpresentpos = round(xpolystart[i])
					#ypresentpos = round(ypolystart[i])
					#if((avoidrepeatedaction <= i) and (xpolystart[i] > 0) and (ypolystart[i]) > 0):
					if((xpolystart[i] > 0) and (ypolystart[i]) > 0):
						polystartx[PLCount] = xpolypresentpos
						polystarty[PLCount] = ypolypresentpos
						polyendx[PLCount] = xpolystart[i]
						polyendy[PLCount] = ypolystart[i]						
						xpolypresentpos = xpolystart[i]#polyendx[PLCount]
						ypolypresentpos = ypolystart[i]#polyendy[PLCount]
						#print("polystartx,polystarty: ",polystartx[PLCount],polystarty[PLCount],"          polyendx,polyendy: ",polyendx[PLCount],polyendy[PLCount])
						PLCount = PLCount + 1
				avoidrepeatedaction=i
				
	if( filereadline == "EOF"): #EOF= End Of File
		arccount = arccount + 1
		break
file.close()
print("file reading is completed")
print("Line count: " , linecount-1)
print("Arc count: ",arccount-1 ,ACount)
print("Circle count: ",circlecount,CCount)
print("Polyline count: ",polylinecount-1, PLCount)
print("layout maximum values of x and y: ",layoutxmax,layoutymax)
#for i in range (1,PLCount):
#	print("polystartx,polystarty: ",polystartx[i],polystarty[i],"          polyendx,polyendy: ",polyendx[i],polyendy[i])

###################### segmented gcode from dxf file function for line,polyline, circle and arc ###########################
def seggcodefromdxf(lx1,ly1,lx2,ly2,linepolycirclearcvar): #"linepolycirclearcvar=1" for line and polyline  or "linepolycirclearcvar=0" for circle and arc
	global gcode
	####################### Vertical Segment #####################
	if lx1 == lx2: #Line is vertical
		#print("Line is vertical")
		if lx1 >= sx1 and lx1 <= sx2: 
			#print("Line is in Segment range")
			if ly1 > ly2: #if line is start with top side and ends with bottom side, to swap the line values
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
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
			elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertica line is spaning below and above the segment
				xstart = lx1 - sx1
				ystart = 0
				xend = lx2 - sx1
				yend = sy2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
			elif ly1 >= sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertical line is start in segment or on segment line and end with out of segment in top side
				xstart = lx1 - sx1
				ystart = ly1 - sy1 
				xend  = lx2 - sx1
				yend = sy2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
			elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 <= sy2: #Vertical line is start in segment or segment line and ens with out of segment in bottom side
				xstart =  lx1 - sx1
				ystart = 0
				xend = lx2 - sx1
				yend  = ly2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"

		else:
			print("Line or Polyline or Circle or Arc may be next segment object")
	########################### Horizantal segment ######################		
	if ly1 == ly2: #Line is Horizantal
		#print("Line is horizontal")	
		if ly1 >= sy1 and ly1 <= sy2 :
			#print("Line is in Segment range")
			if lx1 > lx2: # if line start with left side and ends with right side, to swape the line values
				temp = lx1
				lx1 = lx2
				lx2 = temp

			if lx1 >= sx1 and lx1 <=sx2 and lx2 >= sx1 and lx2 <= sx2: #Horizontal line is within the segment or just covering the entire segment
				xstart = lx1 - sx1
				ystart = ly1 - sy1
				xend = lx2 - sx1
				yend = ly2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle				
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
			elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: #horizantal line is spaning life and right of the segment
				xstart = 0
				ystart = ly1 - sy1
				xend = sx2 - sx1
				yend = ly2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
			elif lx1 >= sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: # Horizontal line is start in segment or segment line and end with out of segment in right side
				xstart = lx1 - sx1
				ystart = ly1 - sy1 
				xend  = sx2 - sx1
				yend = ly2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
			elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 <= sx2:#Horizantal line is start in segment or segment line and end with out of segment in lift side
				xstart =  0
				ystart = ly1 - sy1
				xend = lx2 - sx1
				yend  = ly2 - sy1
				if linepolycirclearcvar == 1 or xstart != xend or ystart != yend:	#remove repeatance value for arc and circle
					gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
					gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
		else:
			print("Line or Polyline or Circle or Arc may be next segment object")


########################## gcode using segmentation ###########################
#layoutxmax = round(layoutxmax)
#layoutymax = round(layoutymax)
xsegmax = int(round(layoutxmax)) // seglen
ysegmax = int(round(layoutymax)) // segwidth

for xsegnum in range(xsegstart,xsegmax):
	for ysegnum in range(ysegstart,ysegmax):
		#print("--------------",ysegnum,"  ",xsegnum)		
		####################### file name format #####################
		xsnum = str(xsegnum)
		ysnum = str(ysegnum)
		xlen = xylenforfile - len(xsnum)
		ylen = xylenforfile - len(ysnum)
		#print(xlen,ylen)
		for i in range(0,xlen):
			xsnum = '0'+ str(xsnum)
		for j in range(0,ylen):
			ysnum = '0' + str(ysnum)
		#print(xsnum,ysnum)		
		file = str(xsnum) + str(ysnum) 
		#file = file_format + str(xsegnum) + file_format +str(ysegnum)
		seg_gcode_file = open((str(file) + '.gcode'),'w')   #file created like xxxyyy.gcode
		print("Segmentaion gcode file: ",seg_gcode_file)
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
		for line in range(1,linecount):
			############# X and Y start and end values ############
			lx1 = round(xstart[line])#3
			ly1 = round(ystart[line])#50
			lx2 = round(xend[line])#70
			ly2 = round(yend[line])#50
			linepolycirclearcvar = 1
			#print("XSATRT :", xstart[line],",ystart: ",ystart[line],"                    xend:",xend[line],",yend:",yend[line])
			seggcodefromdxf(lx1,ly1,lx2,ly2,linepolycirclearcvar);
					
		########################## GCODE for ARC ###################################
		#for ac in range(1,arccount):
		for line in range(1,ACount):
			############# X and Y start and end values ############
			lx1 = round(arcstartx[line])#3
			ly1 = round(arcstarty[line])#50
			lx2 = round(arcendx[line])#70
			ly2 = round(arcendy[line])#50
			linepolycirclearcvar = 0
			#print("arcstartx:",arcstartx[line],",",lx1,"                  arcstarty:",arcstarty[line],",",ly1)
			#print("arcendx:",arcendx[line],",",lx2,"                   arcendy:",arcendy[line],",",ly2)			
			seggcodefromdxf(lx1,ly1,lx2,ly2,linepolycirclearcvar);
					
		######################## GCODE for CIRCLE ################################
		#for cc in range(1,circlecount):
		for line in range(1,CCount):
			############# X and Y start and end values ############
			lx1 = round(circlestartx[line])#3
			ly1 = round(circlestarty[line])#50
			lx2 = round(circleendx[line])#70
			ly2 = round(circleendy[line])#50
			linepolycirclearcvar = 0
			#print("arcstartx:",arcstartx[line],",",lx1,"                  arcstarty:",arcstarty[line],",",ly1)
			#print("arcendx:",arcendx[line],",",lx2,"                   arcendy:",arcendy[line],",",ly2)		
			seggcodefromdxf(lx1,ly1,lx2,ly2,linepolycirclearcvar);
					
		########################GCODE for POLYLINE ##################################
		for pline in range(1,PLCount):
			############# X and Y start and end values ############
			lx1 = round(polystartx[pline])#3
			ly1 = round(polystarty[pline])#50
			lx2 = round(polyendx[pline])#70
			ly2 = round(polyendy[pline])#50
			linepolycirclearcvar = 1
			#print("polystartx:",polystartx[pline],",",lx1,"                  polystarty:",polystarty[pline],",",ly1)
			#print("polyendx:",polyendx[pline],",",lx2,"                   	 polyendy:",polyendy[pline],",",ly2)			
			seggcodefromdxf(lx1,ly1,lx2,ly2,linepolycirclearcvar);								
		
		print(gcode)
		############################# Write gcode to file ###########################
		seg_gcode_file.write(gcode)
		seg_gcode_file.close()
		gcode = ""
		gcodefilescount = gcodefilescount +1
		#time.sleep(1)
print(gcodefilescount , "gcode file are successfully created.")

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
gcode = ""
gcodefilescount = 0
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
		print("Segmentaion gcode file: ",seg_gcode_file)
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
					
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2: #Vertica line is spaning below and above the segment
						xstart = lx1 - sx1
						ystart = 0
						xend = lx2 - sx1
						yend = sy2 - sy1
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 >= sy1 and ly1 < sy2 and ly2 > sy1 and ly2 > sy2:
						xstart = lx1 - sx1
						ystart = ly1 - sy1 
						xend  = lx2 - sx1
						yend = sy2 - sy1 
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif ly1 < sy1 and ly1 < sy2 and ly2 > sy1 and ly2 <= sy2:
						xstart =  lx1 - sx1
						ystart = 0
						xend = lx2 - sx1
						yend  = ly2 - sy1
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"

					
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
					
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2: #horizantal line is spaning life and right of the segment
						xstart = 0
						ystart = ly1 - sy1
						xend = sx2 - sx1
						yend = ly2 - sy1
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 >= sx1 and lx1 < sx2 and lx2 > sx1 and lx2 > sx2:
						xstart = lx1 - sx1
						ystart = ly1 - sy1 
						xend  = sx2 - sx1
						yend = ly2 - sy1 
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
					elif lx1 < sx1 and lx1 < sx2 and lx2 > sx1 and lx2 <= sx2:
						xstart =  0
						ystart = ly1 - sy1
						xend = lx2 - sx1
						yend  = ly2 - sy1
						gcode = gcode + "G00 X" + str(round(xstart)) + " Y" + str(round(ystart)) + "\n"
						gcode = gcode + "G01 X" + str(round(xend)) + " Y" + str(round(yend)) + "\n"
				else:
					print("Line may be next segment object")
			
		print(gcode)
		seg_gcode_file.write(gcode)
		seg_gcode_file.close()
		gcode = ""
		gcodefilescount = gcodefilescount +1
print(gcodefilescount , "gcode file are successfully created.")

'''








'''import os

def function(X1,Y1,X2,Y2):
	#lX1=0
	print("output: ",X1,Y1,X2,Y2)

lx1=1
ly1=2
lx2=3
ly2 =3
function(lx1,ly1,lx2,ly2);'''
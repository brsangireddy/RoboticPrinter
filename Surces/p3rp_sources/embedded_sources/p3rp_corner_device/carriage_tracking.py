# Single Color Code Tracking Example
#
# This example shows off single color code tracking using the OpenMV Cam.
#
# A color code is a blob composed of two or more colors. The example below will
# only track colored objects which have both the colors below in them.

import sensor, image, time, math, pyb
from pyb import UART
"""
# Color Tracking Thresholds (L Min, L Max, A Min, A Max, B Min, B Max)
# The below thresholds track in general red/green things. You may wish to tune them...
thresholds = [(30, 100, 15, 127, 15, 127), # generic_red_thresholds -> index is 0 so code == (1 << 0)
              (30, 100, -64, -8, -32, 32)] # generic_green_thresholds -> index is 1 so code == (1 << 1)
# Codes are or'ed together when "merge=True" for "find_blobs".

#Night-grey
thresholds = [(45, 60, -3, 3, -3, 2), # generic_red_thresholds -> index is 0 so code == (1 << 0)
              (45, 60, -3, 3, -3, 2), # generic_green_thresholds -> index is 1 so code == (1 << 1)
              (45, 60, -3, 3, -3, 2)] # generic_blue_thresholds -> index is 2 so code == (1 << 2)





#Day-color
thresholds = [(65, 75, 25, 45, -10, 10), # generic_red_thresholds -> index is 0 so code == (1 << 0)
              (65, 75, 25, 45, -10, 10), # generic_green_thresholds -> index is 1 so code == (1 << 1)
              (65, 75, 25, 45, -10, 10)] # generic_blue_thresholds -> index is 2 so code == (1 << 2)

"""
#Night-color
thresholds = [(45, 60, 30, 80, 30, 40), # generic_red_thresholds -> index is 0 so code == (1 << 0)
              (45, 60, 30, 80, 30, 40), # generic_green_thresholds -> index is 1 so code == (1 << 1)
              (45, 60, 30, 80, 30, 40)] # generic_blue_thresholds -> index is 2 so code == (1 << 2)
"""
#Default
thresholds = [(30, 100, 15, 127, 15, 127), # generic_red_thresholds -> index is 0 so code == (1 << 0)
              (30, 100, -64, -8, -32, 32), # generic_green_thresholds -> index is 1 so code == (1 << 1)
              (0, 15, 0, 40, -80, -20)] # generic_blue_thresholds -> index is 2 so code == (1 << 2)
"""
#*************************************For LASER Measurement Module**********************************
laser_off_on = [(0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x00,0xC0), #sequence to turn laser OFF
                (0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x01,0xC1)] #sequence to turn laser ON

laser_meas_cmd_seq = [(0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x02,0x23), #OneShotFast mode
                      (0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x01,0x22), #OneShotSlow mode
                      (0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x00,0x21)] #OneShotAuto mode

fast_meas_cmd_seq = [0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x02,0x23] #OneShotFast mode
#fast_meas_cmd_seq = [0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38] #OneShotFast mode

threshold_index = 0;
ldmm = UART(1,19200) #Laser Distance Measurement Module UART interface
#ldmm.init(19200, bits=8, parity=None, stop=1)

ldmm_cmd_sz    = 9
ldmm_result_sz = 13
dm_mode_fast   = 0
dm_mode_slow   = 1
dm_mode_auto   = 2
dist_mm        = 0
off = 0
on  = 1

#***************************For controlling stepper motor functionality*****************************
pm_home_pin  = pyb.Pin("P2", pyb.Pin.IN)#("P2", pyb.Pin.IN)       #pan motor home position detect pin
zfm_endis_pin = pyb.Pin("P3", pyb.Pin.OUT_PP) #Zoom, Focus Motor enable/disable pin

pm_pulse_pin = pyb.Pin("P6", pyb.Pin.OUT_PP)#("P5", pyb.Pin.OUT_PP)#pan motor drive pulse pin
pm_dir_pin   = pyb.Pin("P4", pyb.Pin.OUT_PP)#("P4", pyb.Pin.OUT_PP)#pam motor direction pin
zm_dir_pin = pyb.Pin("P7", pyb.Pin.OUT_PP) #zoom motor direction pin
zm_pulse_pin = pyb.Pin("P5", pyb.Pin.OUT_PP) #zoom motor step/pulse pin
fm_dir_pin = pyb.Pin("P9", pyb.Pin.OUT_PP) #focus motor direction pin
fm_pulse_pin = pyb.Pin("P8", pyb.Pin.OUT_PP) #focus motor step/pluse pin
tm_dir_pin = pyb.Pin("P9", pyb.Pin.OUT_PP)
tm_dir_pin = pyb.Pin("P8", pyb.Pin.OUT_PP)

cw  = 0
ccw = 1

steps_per_deg   = 222.222 #steps for stepper motor to run for rotating 1° degree angle

steps_45_deg    = 10000   #motor steps per 45 degree angle rotation
steps_90_deg    = 20000   #motor steps per 90 degree angle rotation
steps_135_deg   = 30000   #motor steps per 135 degree angle rotation
step_delay      = 100     #microseconds
curr_pos_ang    = 0       #number steps away from home position
curr_pos_dir    = 0       #clockwise (right side of the home position)

track_ang_th    = 50      #max mum angle motor can rotate +/-50 degrees
tilt_ang_th     = 45      #Downward
focus_ang_th    = 10      #To be checked
zoom_ang_th     = 15      #To be checked

track_step_ang  = 0.05     #Angle by which camera rotates in each step while tracking the carriage
tilt_step_ang   = 1.0     #Imaging camera up/down angle to be rotated
focus_step_ang  = 1.0     #focus motor step angle for focus operation
zoom_step_ang   = 1.0     #Zoom motor step angle for zooming operation

net_track_ang   = 0.0
net_tilt_ang    = 0.0
net_zoom_ang    = 0.0
net_focus_ang   = 0.0
net_scan_car_ang = 0.0
#*****************************For Object tracking***************************************************
img_cntr_th     = 5       #pixels
blob_elongation_th = 1.0
start_tracking = False
car_found_dir = ccw
frame_count = 0


#***************************************************************************************************
#Routine to detect home position
#Rotate stepper motor cw/ccw until m_home_pin if high
#***************************************************************************************************
def pan_mot_home():
    global curr_pos_ang, curr_pos_dir
    #print("Initializing motor position")
    curr_pos_ang = 0
    curr_pos_dir = 0
    pm_dir_pin.low()#set direction to clockwise

    for i in range(0,10):
        home_pin_val = pm_home_pin.value()
        #print(home_pin_val)
        if(home_pin_val):
            break
        pm_pulse_pin.high()
        pyb.udelay(step_delay)
        pm_pulse_pin.low()
        pyb.udelay(step_delay)

    for i in range(0,steps_90_deg):
        home_pin_val = pm_home_pin.value()
        if(home_pin_val):
            break
        pm_pulse_pin.high()
        pyb.udelay(step_delay)
        pm_pulse_pin.low()
        pyb.udelay(step_delay)

    pm_dir_pin.high()
    for i in range(0,steps_135_deg):
        home_pin_val = pm_home_pin.value()
        #print(home_pin_val)
        if(home_pin_val):
            break
        pm_pulse_pin.high()
        pyb.udelay(step_delay)
        pm_pulse_pin.low()
        pyb.udelay(step_delay)
    #print("Initializing motor position to home completed")


#***************************************************************************************************
#Rotate motor by given angle in degrees from home position. Coverts degrees to steps and drives
#those many pulses
#***************************************************************************************************
def pan_mot_run(cw_ccw, ang_deg):
    global curr_pos_ang, curr_pos_dir
    curr_pos_dir = cw
    if(cw_ccw == cw):
        pm_dir_pin.low()
        curr_pos_ang = curr_pos_ang + ang_deg
    else:
        pm_dir_pin.high()
        curr_pos_ang = curr_pos_ang - ang_deg

    if(curr_pos_ang < 0):
        curr_pos_ang = -curr_pos_ang
        curr_pos_dir = ccw

    #print(curr_pos_ang)

    step_cnt = round(ang_deg * steps_per_deg)
    #print(step_cnt)

    for i in range(0,step_cnt):
        pm_pulse_pin.high()
        pyb.udelay(step_delay)
        pm_pulse_pin.low()
        pyb.udelay(step_delay)

#***************************************************************************************************
#Rotate focus motor by given number of steps
#***************************************************************************************************
def focus_mot_run(cw_ccw, n_steps):
    zfm_endis_pin.low()#enable zoom, focus motors
    if(cw_ccw == cw):
        fm_dir_pin.low()
    else:
        fm_dir_pin.high()


    for i in range(0,n_steps):
        fm_pulse_pin.high()
        pyb.udelay(step_delay)
        fm_pulse_pin.low()
        pyb.udelay(step_delay)
    zfm_endis_pin.high()#disable zoom, focus motors

#***************************************************************************************************
#Rotate focus motor by given number of steps
#***************************************************************************************************
def zoom_mot_run(cw_ccw, n_steps):
    zfm_endis_pin.low()

    if(cw_ccw == cw):
        zm_dir_pin.low()
    else:
        zm_dir_pin.high()


    for i in range(0,n_steps):
        zm_pulse_pin.high()
        pyb.udelay(step_delay)
        zm_pulse_pin.low()
        pyb.udelay(step_delay)
    zfm_endis_pin.high()

#***************************************************************************************************
#Turn on/off laser
#Write corresponding character sequence to Laser Distance Measurement Module (ldmm) UART
#***************************************************************************************************
def TurnLaserOnOff(on_off):
    global laser_off_on
    for i in range(0,ldmm_cmd_sz):
        ldmm.writechar(laser_off_on[on_off][i])
        #print("%02x"%laser_off_on[on_off][i],end=" ")
    #print("")
    while(True):
        if(ldmm.any()):
            break

    if(ldmm.any()):
        ldmm.read(ldmm_cmd_sz) #for laser on/off command, the result buf size is cmd buf size
#***************************************************************************************************
#Instruct LDMM to measure distance by sending command sequence OneShotFast/OnShotSlow/OneShotAuto
#***************************************************************************************************
def MeasureDistance(mode):
    global laser_meas_cmd_seq, dist_mm,ldmm_result_sz,ldmm_cmd_sz

    for i in range(0,ldmm_cmd_sz):
        #ldmm.writechar(fast_meas_cmd_seq[i])
        ldmm.writechar(laser_meas_cmd_seq[mode][i])
        #print("%02x"%laser_meas_cmd_seq[mode][i],end=" ")
    #print(" ")
    time.sleep(100)
    while(True):
        if(ldmm.any()):
            break
    meas_res = ldmm.read(ldmm_result_sz)
    """
    print(len(meas_res))
    for i in range(0,len(meas_res)):
        print("%02x"%meas_res[i],end=" ")
    print("")
    dist_mm = 0
    """
    if((meas_res[0]==0xaa) and (len(meas_res)==ldmm_result_sz)):
        dist_mm = meas_res[6]<<24|meas_res[7]<<16|meas_res[8]<<8|meas_res[9]
        #print(dist_mm)


#ldmm.writechar(0x55)
#TurnLaserOnOff(on)
#time.sleep(1000)
TurnLaserOnOff(off)
#time.sleep(1000)

#while(True):
#    MeasureDistance(dm_mode_fast)
#    print(dist_mm)

#TurnLaserOnOff(on)
pan_mot_home()

sensor.reset()
#sensor.set_auto_whitebal(False)
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)
sensor.set_auto_gain(False) # must be turned off for color tracking
sensor.set_auto_whitebal(False) # must be turned off for color tracking
clock = time.clock()

intensity = [0] * 320
#lmins = [0] * 160
#lmaxs = [0] * 160
lmms = [0] * 160
lmm_cntr = 0
raising = 1
falling = 0
trend = falling
prev_trend = falling
bm = 0 #blurr measure
ec = 0 #Edge count
ew_th = 30 #edge width threshold
bm_th = 40 #blurr measure threshold

# Only blobs that with more pixels than "pixel_threshold" and more area than "area_threshold" are
# returned by "find_blobs" below. Change "pixels_threshold" and "area_threshold" if you change the
# camera resolution. "merge=True" must be set to merge overlapping color blobs for color codes.
while(True):
    #focus_mot_run(cw, 100)
    #pyb.udelay(step_delay)
    #focus_mot_run(ccw,100)
    #pyb.udelay(step_delay)

    clock.tick()
    img = sensor.snapshot().replace(vflip=True)
    #img.draw_string(30,100,"Processing")

    #img.to_grayscale(False,-1)
    """
    lmm_cntr = 0;
    for i in range(0, len(lmms)):
        lmms[0]

    intensity[0] = img.get_pixel(0,140,False)
    for x in range (1,img.width()-1):
        intensity[x] = img.get_pixel(x,140,False)
        if(intensity[x-1] != intensity[x]):
            if(intensity[x-1] < intensity[x]):
                trend = raising
            else:
                trend = falling

        if(prev_trend != trend):
            lmms[lmm_cntr] = x-1
            prev_trend = trend
            lmm_cntr = lmm_cntr+1

    ec = 0
    bm = 0
    for i in range (1,len(lmms)):
        ew = lmms[i] - lmms[i-1] #edge width
        if(ew > ew_th):
            bm = bm + ew
            ec = ec+1

    if(ec != 0):
        bm = bm//ec #Average of edge widths '//' for floor division

    print(bm)
    """
    #print("intensity: ",intensity)
    #print("local MMs: ",lmms)

    #img.to_rainbow(False,-1,sensor.PALETTE_RAINBOW)
    #img.find_edges(image.EDGE_SIMPLE)
    #print(img.get_histogram())
    #stdev = img.get_statistics().stdev()
    #print("Image size:",img.width(),"X",img.height(),"pixels")
    blobs = img.find_blobs([thresholds[threshold_index]], x_hist_bins_max=30, pixels_threshold=1000, area_threshold=2000, merge=True)
    blobs_found = len(blobs)
    #print(blobs_found)
    if(blobs_found > 0 or start_tracking == True):
        if(start_tracking == False):
            start_tracking = True
            #print("Car found")
            pan_mot_run(car_found_dir,track_step_ang*6)#continue in the same dir to bring blob well into the fov
            if(net_scan_car_ang < 0):
                pan_mot_run(cw,0.5)
            elif(net_scan_car_ang > 0):
                pan_mot_run(ccw,0.5)
        for blob in blobs:
            #print(blob.x_hist_bins()[0][0])
            #print(blob.w(),blob.h())
            #print(blob.pixels(),blob.area())
        #for blob in img.find_blobs(thresholds, pixels_threshold=25, area_threshold=50, merge=True):
            #print(blob.code())
            if blob.code() == 1:#3: # r/g code == (1 << 1) | (1 << 0)
                #print(blob.elongation())
                if blob.elongation() < blob_elongation_th:
                    img.draw_rectangle(blob.rect())
                    img.draw_cross(blob.cx(), blob.cy())
                    img.draw_keypoints([(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=20)
                    diff = blob.cx()-160 #img.width()/2 #Center of the image
                    #print(diff)
                    if(abs(diff)>img_cntr_th):
                        #img.draw_string(30,100,"Tracking")
                        #print("Tracking")
                        if(diff<img_cntr_th):
                            #print("ccw nta: ",net_track_ang)
                            net_track_ang -= track_step_ang
                            if(abs(net_track_ang) < track_ang_th):
                                pan_mot_run(ccw,track_step_ang)
                                #print("sd")#step done
                            else:
                                print("Carriage out of tracking limits.")
                        else:
                            #print("cw nta: ",net_track_ang)
                            net_track_ang += track_step_ang
                            if(abs(net_track_ang) < track_ang_th):
                                pan_mot_run(cw,track_step_ang)
                            else:
                                print("Carriage out of tracking limits.")
                    else:
                        img.draw_string(30,30,"Measuring Distance")
                        #MeasureDistance(dm_mode_fast)
                        #print(dist_mm)
                        #print("")
                else:
                    print("Elongation > ", blob_elongation_th)

            #print(blob.cx(),blob.cy())
    else:
        #print("Searching")
        #img.draw_string(30,100,"Searching")
        if(net_scan_car_ang > -track_ang_th):
            net_scan_car_ang -= track_step_ang
            pan_mot_run(ccw,track_step_ang)
            car_found_dir = ccw
        elif(net_scan_car_ang < track_ang_th):
            net_scan_car_ang += track_step_ang
            pan_mot_run(cw,track_step_ang)
            car_found_dir = cw


    #print(blob.cy())
    #print(clock.fps())


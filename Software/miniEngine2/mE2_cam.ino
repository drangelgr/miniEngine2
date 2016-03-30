/*

    Author: Airic Lenz
    Year of release: 2016
    
    See www.airiclenz.com for more information

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/



////////////////////////////////////////////////////////
//                                                    //
//  C A M E R A   V A R I A B L E S                   //
//                                                    //
////////////////////////////////////////////////////////


// B0 = camera is exposing
// B1 = camera post delay active
// B2 = camera is focusing
// B3 = 
// B4 = 
// B5 = 
// B6 = 
// B7 = camera type
uint8_t cam_status = B0;


// variables for tracking the actual exposure
uint32_t cam_start_time;

// variables for tracking the shoot count
uint16_t cam_shoot_count;




// EXPOSURE VALUES
typedef struct cameraExposure {
   uint16_t  exposure;
   char      *name;
};

#define CAM_EXPOSURE_COUNT 17

const struct cameraExposure cam_exposure_values[CAM_EXPOSURE_COUNT] = {
  {   50, "<= 1/20s"},  // 0
  {   66,    "1/15s"},  // 1
  {   77,    "1/13s"},  // 2
  {  100,    "1/10s"},  // 3
  {  125,     "1/8s"},  // 4
  {  166,     "1/6s"},  // 5
  {  200,     "1/5s"},  // 6
  {  250,     "1/4s"},  // 7
  {  300,     "0.3s"},  // 8
  {  400,     "0.4s"},  // 9
  {  500,     "0.5s"},  // 10
  {  600,     "0.6s"},  // 11
  {  800,     "0.8s"},  // 12
  { 1000,     "1.0s"},  // 13
  { 1300,     "1.3s"},  // 14
  { 1600,     "1.6s"},  // 15
  { 2000,     "2.0s"}   // 16
};

// this variable is used to go through the exposures
unsigned int cam_exposure_index = 3; // 1/10 sec









// ============================================================================
boolean cam_isCameraExposingFlag()       { return isBit(cam_status, BIT_0); }
void    cam_setCameraExposingFlag()      { setBit(cam_status, BIT_0); }
void    cam_deleteCameraExposingFlag()   { deleteBit(cam_status, BIT_0); }

boolean cam_isPostDelayActiveFlag()      { return isBit(cam_status, BIT_1); }
void    cam_setPostDelayActiveFlag()     { setBit(cam_status, BIT_1); }
void    cam_deletePostDelayActiveFlag()  { deleteBit(cam_status, BIT_1); }

boolean cam_isCameraFocusingFlag()       { return isBit(cam_status, BIT_2); }
void    cam_setCameraFocusingFlag()      { setBit(cam_status, BIT_2); }
void    cam_deleteCameraFocusingFlag()   { deleteBit(cam_status, BIT_2); }

void    cam_toggleCameraType()           { toggleBit(cam_status, BIT_7); }  
boolean cam_isCameraType()               { return isBit(cam_status, BIT_7); } 


// ============================================================================
boolean cam_isCameraWorking() {
    
  return isBit(cam_status, B00000111);
  
}



// ============================================================================
// initializes the camera
// ============================================================================
void cam_init() {
  
  // set pin modes
  pinMode(PIN_CAM_SHUTTER, OUTPUT);
  pinMode(PIN_CAM_FOCUS, OUTPUT);
  pinMode(PIN_CAM_ALT_SHUTTER, OUTPUT);
  pinMode(PIN_CAM_ALT_FOCUS, OUTPUT);

  digitalWrite(PIN_CAM_SHUTTER, LOW);
  digitalWrite(PIN_CAM_FOCUS, LOW);
  digitalWrite(PIN_CAM_ALT_SHUTTER, LOW);
  digitalWrite(PIN_CAM_ALT_FOCUS, LOW);
  
}


// ============================================================================
void cam_process() {
  
  // are we focusing?
  if (cam_isCameraFocusingFlag()) {
    
    // is the time over?
    if ((cam_start_time + cam_focus) <= millis()) {
      
      // stop focusing
      digitalWrite(PIN_CAM_FOCUS, LOW);
      
      // remove the focusing flag
      cam_deleteCameraFocusingFlag();
      
      // remember the time we started
      cam_start_time = millis();
      
      // start exposing
      cam_trigger(); 
        
      // set the exposure flag
      cam_setCameraExposingFlag();
      
      // leave
      return;
    }
    
  }
  
  // are we exposing?
  else if (cam_isCameraExposingFlag()) {
     
    // is the time over?
    if ((cam_start_time + cam_exposure) <= millis()) {
        
      // de-trigger the camera
      cam_stop();
            
      // count the shoot count up
      cam_shoot_count++;        
      
      // request a shoot-count repaint
      uicore_setRepaintShootCount();
             
      // if there is a camera post delay, start it
      if (cam_post_delay > 0) {
        
        cam_setPostDelayActiveFlag(); 
        cam_start_time = millis();
        
        // leave
        return;
      }
             
    }  
     
  } 
   
  // are we in post delay?
  else if (cam_isPostDelayActiveFlag()) {
    
    // is the time over?
    if ((cam_start_time + cam_post_delay) <= millis()) {
      
      cam_deletePostDelayActiveFlag(); 
      return;   
    }
          
  }
     
}


// ============================================================================
void cam_start() {
  
  // if the camera is not exposing and not in post delay
  if (!isBit(cam_status, B00000111)) {
    
    // set the values for the focus / exposure 
    cam_start_time = millis();
    
    // are we supposed to focus before shooting?
    if (cam_focus > 0) {
      
      // activate focusing
      digitalWrite(PIN_CAM_FOCUS, HIGH);
        
      // set the focus flag
      cam_setCameraFocusingFlag();
      
    } 
    // else trigger immediatelly
    else {
      // press the button
      cam_trigger(); 
        
      // set the exposure flag
      cam_setCameraExposingFlag();
    }
    
        
    
      
  }
  
}


// ============================================================================
void cam_trigger() {
  
  // trigger the main camera
  
  // Nikon Mode
  if (isBit(cam_status, BIT_7)) {
    digitalWrite(PIN_CAM_SHUTTER, HIGH);
    digitalWrite(PIN_CAM_FOCUS, LOW);  
  } 
  // Canon Mode
  else {
    digitalWrite(PIN_CAM_SHUTTER, HIGH);
    digitalWrite(PIN_CAM_FOCUS, HIGH);
  }  
  
}


// ============================================================================
void cam_stop() {
  
  digitalWrite(PIN_CAM_SHUTTER, LOW);
  digitalWrite(PIN_CAM_FOCUS, LOW);  
  
  // delete the camere-is-active flag
  cam_deleteCameraExposingFlag();
  cam_deleteCameraFocusingFlag();
}



// ============================================================================
void cam_resetShootCount() {
  cam_shoot_count = 0;
}

// ============================================================================
uint16_t cam_getShootCount() {
  return cam_shoot_count;
}

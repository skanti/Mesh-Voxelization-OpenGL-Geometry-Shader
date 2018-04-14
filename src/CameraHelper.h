#include <math.h>
#include <stdio.h>


namespace Camera {
	#define EPSILON 0.0001
	
	/** Current camera lookat *point*. A lookat vector is created by
	 * subtracting the lookat point from the camera position. */
	static float cam_lookat[3] = {0, 0, 0};
	/** Current camera position */
	static float cam_position[3] = {0, 0, -1};
	/** Current camera up vector. Since mousemove does not support roll,
	 * this value is only changed when a user specifically sets it with
	 * mousemove_set() or mousemove_setVec() */
	static float cam_up[3] = { 0.0f, 1.0f, 0.0f };

	static float pitch = 0;	
	static float pitch_down;	
	static float yaw = M_PI;	
	static float yaw_down;	

	static float rot_rate = 0.01f;  /**< amount to scale rotations  */
	static float trans_rate = 0.001f;/**< amount to scale translations */
	static float scroll_rate = 0.005f;/**< amount to scale translations */
	
	/** Currently pressed mouse button. -1=no button pressed, 0=left,
	 * 1=middle, 2=right */
	static int cur_button = -1;
	static int last_x; /**< Last X coordinate of the mouse cursor */
	static int last_y; /**< Last Y coordinate of the mouse cursor */
	static float cam_lookat_down[3]; /**< The lookat vector when the mouse button was last pressed down */
	static float cam_position_down[3]; /**< The camera position when the mouse button was last pressed down */
	
	/** Internal function to move camera along the look at vector
	 * @param dy Amount to translate camera down the lookVec
	 * @param lookVec Vector pointing where camera is looking
	 */
	static inline void make_rotx(float alpha, float *A) {
		float c = std::cos(alpha);
		float s = std::sin(alpha);
		A[0*3 + 0] = 1;
		A[0*3 + 1] = 0;
		A[0*3 + 2] = 0;
		A[1*3 + 0] = 0;
		A[1*3 + 1] = c;
		A[1*3 + 2] = s;
		A[2*3 + 0] = 0;
		A[2*3 + 1] = -s;
		A[2*3 + 2] = c;
	}
		
	static inline void make_roty(float alpha, float *A) {
		float c = std::cos(alpha);
		float s = std::sin(alpha);
		A[0*3 + 0] = c;
		A[0*3 + 1] = 0;
		A[0*3 + 2] = -s;
		A[1*3 + 0] = 0;
		A[1*3 + 1] = 1;
		A[1*3 + 2] = 0;
		A[2*3 + 0] = s;
		A[2*3 + 1] = 0;
		A[2*3 + 2] = c;
	}

	static inline void vec3f_copy(float result[3], const float a[3]) {
	    result[0] = a[0]; result[1] = a[1]; result[2] = a[2];
	}
	
	static inline void vec3f_set(float  v[3], float  a, float  b, float  c) {
	    v[0]=a; v[1]=b; v[2]=c;
	}
	
	static inline float vec3f_norm(float src[3]){
	    return std::sqrt(src[0]*src[0] + src[1]*src[1] + src[2]*src[2]);
	}
	
	static inline void vec3f_normalize(float src[3]){
	    float norm = vec3f_norm(src);
	    src[0] = src[0]/norm; src[1] = src[1]/norm; src[2] = src[2]/norm;
	}
	
	static inline void vec3f_add(float result[3], const float a[3], const float b[3]){
	    result[0] = a[0] + b[0]; result[1] = a[1] + b[1]; result[2] = a[2] + b[2];
	}
	
	static inline void vec3f_mul(float result[3], const float a[3], float s){
	    result[0] = s*a[0]; result[1] = s*a[1]; result[2] = s*a[2];
	}
	
	static inline void vec3f_sub(float result[3], const float a[3], const float b[3]){
	    result[0] = a[0] - b[0]; result[1] = a[1] - b[1]; result[2] = a[2] - b[2];
	}
	
	static inline void vec3f_cross(float result[3], const float A[3], const float B[3]){
		float temp[3];
		temp[0] = A[1]*B[2] - A[2]*B[1];
		temp[1] = A[2]*B[0] - A[0]*B[2];
		temp[2] = A[0]*B[1] - A[1]*B[0];
		vec3f_copy(result, temp);
	}
	
	static inline void sgemv3(float y[3], const float A[9], const float x[3]){
	    y[0] = A[0*3 + 0]*x[0] + A[1*3 + 0]*x[1] + A[2*3 + 0]*x[2];
	    y[1] = A[0*3 + 1]*x[0] + A[1*3 + 1]*x[1] + A[2*3 + 1]*x[2];
	    y[2] = A[0*3 + 2]*x[0] + A[1*3 + 2]*x[1] + A[2*3 + 2]*x[2];
	}
	
	void mat3f_rotateAxisVec(float result[9], float degrees, float axis[3]) {
		float angle = (float)(degrees * M_PI/180.0f);
		float c = std::cos(angle), s = std::sin(angle);
		float t = 1-c;
	
		if(c > .9)
			t = 2.0f * sinf(angle/2.0f)*sinf(angle/2.0f);
	
		float length = vec3f_norm(axis);
	
		float x = axis[0]/length;	float y = axis[1]/length;	float z = axis[2]/length;
		result[0] = x*x*t+c;	result[3] = x*y*t-z*s;	result[6] = x*z*t+y*s;
		result[1] = y*x*t+z*s;	result[4] = y*y*t+c;	result[7] = y*z*t-x*s;
		result[2] = z*x*t-y*s;	result[5] = z*y*t+x*s;	result[8] = z*z*t+c;
	}
	
	static inline float vec3f_normSq(const float A[3]){
	    return A[0]*A[0] + A[1]*A[1] + A[2]*A[2];
	}
	
	void mousemove_scroll(int dy, const float lookVec[3]){
	    // Move the camera and lookat point along the look vector
	    for(int i=0; i<3; i++)
	    {
	        float offset = lookVec[i] * -dy * scroll_rate;
	        cam_position[i] = cam_position_down[i] - offset;
	        //cam_lookat[i]   = cam_lookat_down[i]   - offset;
	    }
	}
	
	
	/** This function should be called whenever a mouse button is pressed
	 * and is automatically called by mousemove_glfwMouseButtonCallback()
	 * and mousemove_glutMouseFunc()
	 *
	 * @param down 1 if the button is being pressed down or 0 if the button is being released.
	 * @param leftMidRight 0=left button is pressed, 1=middle button is pressed, 2=right button is pressed, 3=scoll up, 4=scroll down, -1 no button is pressed.
	 * @param x The x coordinate of the mouse cursor when the button is pressed (or amount to scroll horizontally if scrolling event).
	 * @param y The y coordinate of the mouse cursor when the button is pressed (or amount to scroll vertically if scrolling event).
	 */
	void mousemove_buttonPress(int down, int leftMidRight, int x, int y)
	{
	    if(down)
	    {
	        cur_button = leftMidRight;
	
	        last_x = x;
	        last_y = y;
	
	        // Store camera position & lookat when mouse button is pressed
	        vec3f_copy(cam_lookat_down, cam_lookat);
	        vec3f_copy(cam_position_down, cam_position);
			pitch_down = pitch;
			yaw_down = yaw;
	        if(leftMidRight>2)
	        {
	            float lookAt[3];
	            // Calculate a new vector pointing from the camera to the
	            // look at point and normalize it.
	            vec3f_sub(lookAt, cam_lookat_down, cam_position_down);
	            if(cur_button == 3) // scroll up (zoom in)
	                mousemove_scroll(y,lookAt);
	            else // cur_button = 4
	                mousemove_scroll(y,lookAt);
	        }
	    }
	    else
	        cur_button = -1;
	}

	static void set_pose_by_yaw_and_pitch(float yaw, float pitch, float t[3], float distance, float offset[3]) {
		t[0] = offset[0] + distance*std::sin(yaw)*std::cos(pitch);
		t[1] = offset[1] - distance*std::sin(pitch);
		t[2] = offset[2] + distance*std::cos(yaw)*std::cos(pitch);
	}
	
	/** This function should be called whenever a mouse cursor motion
	 * occurs and is automatically called by
	 * mousemove_glfwCursorPosCallback() and mousemove_glutMotionFunc()
	 *
	 * @param x The x coordinate of the mouse cursor.
	 * @param y The y coordinate of the mouse cursor.
	 * @return 1 if the scene needs to be redawn, 0 otherwise.
	 */
	int mousemove_movement(int x, int y)
	{
	    if(cur_button == -1)
	        return 0;
	
	    /* Distance cursor has moved since last press. */
	    int dx = x-last_x;
	    int dy = y-last_y;
	    /* Vectors to store our orthonormal basis. */
	    float f[3], r[3], u[3];
	
	    // Calculate a new vector pointing from the camera to the
	    // look at point and normalize it.
	    vec3f_sub(f, cam_lookat_down, cam_position_down);
	    vec3f_normalize(f);
	
	    // Get our up vector
	    vec3f_copy(u, cam_up);
	
	    // Get a right vector based on the up and lookat vector.
	    vec3f_cross(r, f, u);
	    vec3f_normalize(r);
		vec3f_cross(u, r, f);
		vec3f_normalize(u);
	
	    switch(cur_button)
	    {
	        case 0: // left mouse button - translate left/right or up/down
	
	            /* Translate the camera along the right and up vectors
	             * appropriately depending on the type of mouse movement.  */
	            for(int i=0; i<3; i++) {
	                float offset = r[i]*dx*trans_rate - u[i]*dy*trans_rate;
	                cam_position[i] = cam_position_down[i] - offset;
	                cam_lookat[i]   = cam_lookat_down[i]   - offset;
	            }
	            break;
	        case 2: // right mouse button - rotate
				pitch = -dy*rot_rate + pitch_down;
				yaw = -dx*rot_rate + yaw_down;
				
				if (pitch >= M_PI*0.49)
					pitch = M_PI*0.49;
				else if (pitch <= -M_PI*0.49)
					pitch = -M_PI*0.49;
				
	    		vec3f_sub(f, cam_lookat_down, cam_position_down);
				float distance = vec3f_norm(f);
				//printf("d: %f lx: %f ly: %f lz: %f px: %f py: %f pz: %f\n", distance, cam_lookat[0], cam_lookat[1], cam_lookat[2], cam_position[0], cam_position[1], cam_position[2]);
				set_pose_by_yaw_and_pitch(yaw, pitch, cam_position, distance, cam_lookat);
	            break;
	    }
	
	    return 1;
	}
	
	
	
	/** A callback function suitable for use with
	 * glfwSetMouseButtonCallback(). Typically, you would call
	 * glftSetMouseButtonCallback(window, mousemove_glfwMouseButtonCallback) to
	 * tell GLFW to call this function whenever a mouse button is
	 * pressed.
	 *
	 * @see mousemove_buttonPress() mousemove_glutMouseFunc()
	 */
	void mousemove_glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
	{
	    double x,y;
	    glfwGetCursorPos(window, &x, &y);
	
	    int leftMidRight = -1;
	    switch(button)
	    {
	        case GLFW_MOUSE_BUTTON_LEFT:   leftMidRight = 0;  break;
	        case GLFW_MOUSE_BUTTON_MIDDLE: leftMidRight = 1;  break;
	        case GLFW_MOUSE_BUTTON_RIGHT:  leftMidRight = 2;  break;
	        default:                       leftMidRight = -1; break;
	    }
	    mousemove_buttonPress(action==GLFW_PRESS, leftMidRight, (int)x, (int)y);
	}
	
	
	/** A callback function suitable for use with
	 *  glfwSetCursorPosCallback(). Typically, you would call
	 *  glfwSetCursorPosCallback(window, mousemove_glfwCursorPosCallback) to tell
	 *  GLFW to call this function whenever a mouse is moved while a mouse
	 *  button is pressed.
	 *
	 * @param window The GLFW window we are working with.
	 * @param x The x coordinate of the mouse cursor.
	 * @param y The y coordinate of the mouse cursor.
	 *
	 * @see mousemove_movement() mousemove_glutMotionFunc()
	 */
	void mousemove_glfwCursorPosCallback(GLFWwindow *window, double x, double y)
	{
	    mousemove_movement(x,y);
	}
	
	/** A callback function suitable for use with
	 * glfwSetScrollCallback(). Typically, you would call
	 * glfwSetScrollCallback(window, mousemove_glfwCursorPosCallback) to
	 * tell GLFW to call this function whenever a scroll event (mouse
	 * scroll wheel, touchpad scroll, etc) occurs.
	 *
	 * @param window The GLFW window we are working with.
	 * @param xoff The change in x (0 if scroll is vertical)
	 * @param yoff The change in y (0 if scroll is horizontal)
	 */
	void mousemove_glfwScrollCallback(GLFWwindow *window, double xoff, double yoff)
	{
	    // msg(MSG_INFO, "glfw scroll %f %f\n", xoff, yoff);
	    if(yoff > 0)
	        mousemove_buttonPress(1, 3, 0, (int)(yoff*10));
	    else if(yoff < 0)
	        mousemove_buttonPress(1, 4, 0, (int)(yoff*10));
	}
}


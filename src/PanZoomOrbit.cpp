/*
 * PanZoomOrbit.cpp
 *
 *  Created on: Jul 25, 2020
 *      Author: Admin
 */

/*Header Inclusions */
#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

//GLM Math Header Inclusions */
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std; // Standard namespace
#define GLSL(Version, Source) "#version " #Version "\n" #Source

#define WINDOW_TITLE "Panning Zooming & Orbiting a Cube"


/* Variable declarations for shader, window size initialization, buffer and array objects*/
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, VAO;

GLfloat cameraSpeed = 0.0005f; //Movement speed per frame
GLchar currentKey; //Will store key pressed
int keymod;  //check for alt

GLfloat scale_by_y=2.0f;
GLfloat scale_by_z=2.0f;
GLfloat scale_by_x=2.0f;

GLfloat lastMouseX = 400, lastMouseY = 300;  //locks mouse cursor at the center of the screen
GLfloat mouseXoffset, mouseYoffset , yaw = 0.0f ,pitch = 0.0f; //mouse offset , yaw and pitch variables
GLfloat sensitivity = 0.01f; //used for mouse and camera sensitivity
bool mouseDetected = true; //initially true when mouse is detected

bool rotate = false;

bool checkMotion = false;


bool checkZoom = false;

//Global vector declarations
glm::vec3 CameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial camera position. Placed units in Z
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); //Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -1.0f); //Temporary z unit vector
glm::vec3 front;  //temporary z unit vector for mouse

/*Function prototypes */
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UMouseMove(int x , int y);
void OnMouseClicks(int button, int state, int x , int y);
void onMotion(int x, int y);


/* Vector Shader Source Code */

/* Vertex Shader Source Code*/
const GLchar * vertexShaderSource ="#version 400 core\n"
		// Vertex data from Vertex Attrib Pointer 0
        "layout(location = 0) in vec3 position;"
		// Color data from Vertex Attrib Pointer 1
        "layout(location = 1) in vec3 color;"
		//variable to transfer color data to the fragment shader
        "out vec3 mobileColor;"
		//Global variables for the transform matrices
        "uniform mat4 model;"
        "uniform mat4 view;"
        "uniform mat4 projection;"
        "void main()\n"
        "{\n"
		// transforms vertices to clip coordinates
        "gl_Position =  projection * view * model * vec4(position, 1.0f);"
		// references incoming color data
        "mobileColor = color;"
        "}\n";

/* Fragment Shader Source Code*/
const GLchar * fragmentShaderSource ="#version 400 core\n"
        "in vec3 mobileColor;" // Variable to hold incoming color data from vertex shader
        "out vec4 gpuColor;"  // Variable to pass color data to the GPU
        "void main()\n"
        "{\n"
        "gpuColor = vec4(mobileColor, 1.0);"  // Sends color data to the GPU for rendering
        "}\n";



/* Fragment Shader Source Code */

/* Main Program */
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			return -1;
		}

		UCreateShader();
		UCreateBuffers();

		// Use the Shader program
		glUseProgram(shaderProgram);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

	glutDisplayFunc(URenderGraphics);


	glutPassiveMotionFunc(UMouseMove); // detect mouse move

    glutMotionFunc(onMotion);


	glutMouseFunc(OnMouseClicks);


	glutMainLoop();


	// Destroys Buffer objects once used
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	return 0;
}


/* Resizes the Window*/
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);

}

/* Renders graphics */
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	glBindVertexArray(VAO); // Active the Vertex Array object before rendering and transforming them


	//* Create Movement Logic */

    CameraForwardZ = front;

	// Transforms the Object
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the object at the center of the viewport
	model = glm::rotate(model, 45.0f, glm::vec3(0.0, 1.0f, 0.0f)); // Rotate the object 45 degrees on the X
	model = glm::scale(model, glm::vec3(scale_by_x,scale_by_y,scale_by_z)); // Increase the object size by a scale of 2

	//Transforms the camera
	glm::mat4 view;
	view = glm::lookAt(CameraForwardZ, CameraPosition, CameraUpY);

	// Creates a perspective projection
	glm::mat4 projection;
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
	GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glutPostRedisplay();

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0); // Deactivates the Vertex Array Object
	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame
}


/* Creates the Shader program */
void UCreateShader()
{
	//Vertex shader
	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // Attaches the Vertex shader to the source code
	glCompileShader(vertexShader); // Compiles the Vertex shader

	// Fragment shader
	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); //Attaches the Fragment shader to the source code
	glCompileShader(fragmentShader); // Compiles the Fragment shader

	//Shader program
	shaderProgram = glCreateProgram(); //Create the Shader program and returns an id
	glAttachShader(shaderProgram, vertexShader); //Attach Vertex shader to the Shader program
	glAttachShader(shaderProgram, fragmentShader);; // Attach Fragment shader to the shader program
	glLinkProgram(shaderProgram); // Link vertex and fragment shader to shader program

	//Delete the Vertex and Fragment shaders once linked
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}


void UCreateBuffers()
{

	GLfloat vertices[] = {
								 //Positions    	//Color

			                                 //p1
								-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
								 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
								 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
								 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
								-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
								-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

								              //p2
								 -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
								  0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
								  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
								  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
								 -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
								 -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,

								             //p3
								 -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
								 -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
								 -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
								 -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
								 -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
								 -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,

								 	 	 	 //p4
								 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
								 0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
								 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
								 0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
								 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
								 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f,

								 	 	 	 //p5
								-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
								 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
								 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
								 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
								-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
								-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,

								-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
								 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
								 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
								 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
								-0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
								-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
};


	//Generate buffer ids
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Active the vertex Array object before binding and setting any VBOs and vertex Attribute Pointers
	glBindVertexArray(VAO);

	// Active the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //Copy vertices to VBO


	//Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // Enable vertex attribute


	//Set attribute pointer 1 to hold Color data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // Enable vertex attribute


	glBindVertexArray(0); // Deactivates the VAO which is good practice


}


void UMouseMove(int x, int y){

        front.x = 10.0f * cos(yaw);
        front.y = 10.0f * sin(pitch);
        front.z = sin(yaw) * cos(pitch) * 10.0f;

}

void onMotion(int curr_x, int curr_y) {

	//if left alt and mouse down are set
	if(checkMotion){

	//gets the direction the mouse was moved
	   mouseXoffset = curr_x - lastMouseX;
	   mouseYoffset = lastMouseY - curr_y;

		  //updates with new mouse coorditaes
		   lastMouseX = curr_x;
		   lastMouseY = curr_y;

		   //Applies sensitivity to mouse direction
		   mouseXoffset *= sensitivity;
		   mouseYoffset *= sensitivity;


           //get the direction of the mouse
		   //if there is changes in yaw, then it is moving along X
		   if(yaw != yaw+mouseXoffset && pitch == pitch+mouseYoffset){

			   //INCREAMENT yaw
			   yaw += mouseXoffset;



			   //else movement in y
		   }else if(pitch != pitch+mouseYoffset && yaw == yaw+mouseXoffset ){


			   //increament y to move vertical
			    pitch += mouseYoffset;

		   }


		   front.x = 10.0f * cos(yaw);
		   front.y = 10.0f * sin(pitch);
		   front.z = sin(yaw) * cos(pitch) * 10.0f;


	}

	//check if user is zooming, alt, right mouse button and down

	if(checkZoom){


		//determine the direction of the , whether up or down
                    if(lastMouseY > curr_y){

                    	//increament scale values
                    	       scale_by_y += 0.1f;
                    		   scale_by_x += 0.1f;
                    		   scale_by_z += 0.1f;

                    		   //redisplay
                    		   glutPostRedisplay();


                    }else{

                    	//decreament scale values, zoom in
                       scale_by_y -= 0.1f;
					   scale_by_x -= 0.1f;
					   scale_by_z -= 0.1f;

					   //control zoom in size
					   if(scale_by_y < 0.2f){

						   scale_by_y = 0.2f;
						   scale_by_x = 0.2f;
						   scale_by_z = 0.2f;
					   }

					   glutPostRedisplay();
                    }

                    //update x and y
                    lastMouseY = curr_y;
                    lastMouseX = curr_x;
	}
}

void OnMouseClicks(int button, int state, int x, int y) {


   keymod = glutGetModifiers(); // checks for modifier keys like alt, shif and ctrl

   checkMotion = false; //set checkMotion to false

   //check if button is left, and mod is alt and state is down, all should be true
   if(button == GLUT_LEFT_BUTTON && keymod == GLUT_ACTIVE_ALT && state == GLUT_DOWN) {

	   //if true then set motion true
      checkMotion = true;

      //zooming to be false
      checkZoom = false;


   }else if(button == GLUT_RIGHT_BUTTON && keymod == GLUT_ACTIVE_ALT && state == GLUT_DOWN){

	   //zoom to be true and motion to be false
	   checkMotion = false;
	   checkZoom = true;
   }
}




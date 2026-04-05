#include <GLFW/glfw3.h>

int main()
{
    // Initialisation
    if (!glfwInit())
        return -1;

    // Création de la fenêtre
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW Minimal", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Contexte OpenGL
    glfwMakeContextCurrent(window);

    // Boucle principale
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Nettoyage
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
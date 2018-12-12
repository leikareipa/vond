/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Vond is a software-based hybrid voxel/polygon 3d landscape renderer, akin in
 * spirit to Novalogic's Voxel Space engine.
 *
 * It's currently under development.
 *
 */

#include <QElapsedTimer>
#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include "../src/config_file_read.h"
#include "../src/display.h"
#include "../src/render.h"
#include "../src/common.h"
#include "../src/camera.h"
#include "../src/image.h"
#include "../src/ui.h"

// Set to !0 when the user wants to exit the program.
static int PROGRAM_EXIT_REQUESTED = 0;

static void init_system(void)
{
    DEBUG(("Initializing the program..."));

    kd_acquire_display(1280, 800, "\"Vond\" by Tarpeeksi Hyvae Soft");

    return;
}

static void release_system(void)
{
    DEBUG(("Releasing the program..."));

    kd_release_display();

    kmem_deallocate_memory_cache();

    return;
}

// Call to ask the program to terminate, possibly with the given  exit code. That
// the program honors this request at a particular time isn't guaranteed.
//
void kmain_request_program_exit(const int exitCode)
{
    DEBUG(("Main has been asked to terminate the program with exit code %d.", exitCode));

    if (exitCode == EXIT_FAILURE)
    {
        exit(EXIT_FAILURE);
    }

    PROGRAM_EXIT_REQUESTED = true; /// For now, ignore the given exit code and just set to signal a generic, successful exit.

    return;
}

// If the exit condition is !false, the program is about to shut down or may even
// already be in the process of doing so.
//
bool kmain_program_is_exiting(void)
{
    return bool(PROGRAM_EXIT_REQUESTED);
}

int main(void)
{
    init_system();

    DEBUG(("Entering the main loop..."));
    {
        // Grab room for the render buffers. Note that the resolution given determines
        // the render resolution, which will then be upscaled to the resolution of the
        // host window.
        framebuffer_s framebuffer(320, 200, 32);

        // Load the landscape heightmap and texture map, as well as a polygon object
        // for testing.
        /// TODO: In the future, asset initialization will be handled somewhere other than here.
        image_s heightmap(QImage("terrain_heightmap.png"), "Terrain heightmap image");
        image_s texmap(QImage("terrain_texture.png"), "Terrain texturemap image");
        std::vector<triangle_s> tris2 = kmesh_mesh_triangles("untitled.vmf");

        /// TODO: In the future, camera initialization will be handled somewhere other than here.
        camera_s camera;
        camera.pos = {512, 120, 512};
        camera.orientation = {0.5, -0.3, 0};
        camera.zoom = 1;
        camera.fov = 65;

        while (!PROGRAM_EXIT_REQUESTED)
        {
            static std::deque<uint> fps;
            static uint avgFPS = 0;
            static uint frameCnt = 0;

            uint renderTime, totalTime;
            QElapsedTimer tim;
            tim.start();

            // Prepare for the new frame.
            {
                krend_clear_framebuffer(&framebuffer);
                ktext_clear_ui_text_entries();
                kinput_reset_input_state();
            }

            // Create the new frame.
            {
                ktext_add_ui_text(std::string("FPS: ") + std::to_string(avgFPS), {10, 20});

                kd_update_input(&camera);

                kr_draw_landscape(heightmap, texmap, camera, &framebuffer);
                kr_draw_triangles(tris2, camera, &framebuffer);

                renderTime = tim.elapsed();
            }

            // Paint the new frame to screen.
            {
                kd_update_display(*framebuffer.canvas);

                totalTime = tim.elapsed();
            }

            // Handle any new user input.
            /// TODO: Implement proper input handling. This works for testing, though.
            {
                if (kinput_is_moving_forward())
                {
                    vector3_s dir = vector3_s(0, 0, 1);
                    dir *= RotationMatrix(0, camera.orientation.y, 0) * RotationMatrix(camera.orientation.x, 0, 0);
                    dir = dir.normalized();
                    camera.pos.x += dir.x;
                    camera.pos.y += dir.y;
                    camera.pos.z += dir.z;
                }
                else if (kinput_is_moving_backward())
                {
                    vector3_s dir = vector3_s(0, 0, -1);
                    dir *= RotationMatrix(0, camera.orientation.y, 0) * RotationMatrix(camera.orientation.x, 0, 0);
                    dir = dir.normalized();
                    camera.pos.x += dir.x;
                    camera.pos.y += dir.y;
                    camera.pos.z += dir.z;
                }
            }

            // Statistics.
            {
                const uint curFPS = (1000 / totalTime);

                // Calculate the average.
                fps.push_back(curFPS);
                if (fps.size() > 40)
                {
                    fps.pop_front();
                }
                avgFPS = (std::accumulate(fps.begin(), fps.end(), 0) / fps.size());

                // Print the current FPS into the console.
                if (frameCnt % 64 == 0)
                {
                    DEBUG(("FPS: %d/%d", ((renderTime == 0)? 999 : (1000 / renderTime)), ((totalTime == 0)? 999 : (1000 / totalTime))));
                }
                frameCnt++;
            }
        }
    }

    release_system();

    DEBUG(("Bye."));
    return EXIT_SUCCESS;
}

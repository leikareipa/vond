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
#include "auxiliary/config_file_read.h"
#include "auxiliary/display.h"
#include "vond/render.h"
#include "vond/assert.h"
#include "vond/camera.h"
#include "vond/image.h"
#include "vond/image_mosaic.h"
#include "auxiliary/ui.h"
#include "auxiliary/ui/input.h"

// Set to !0 when the user wants to exit the program.
static int PROGRAM_EXIT_REQUESTED = 0;

static void init_system(void)
{
    printf("Initializing the program...\n");

    kd_acquire_display(1600, 900, "\"Vond\" by Tarpeeksi Hyvae Soft");

    return;
}

static void release_system(void)
{
    printf("Releasing the program...\n");

    kd_release_display();

    return;
}

// Call to ask the program to terminate, possibly with the given  exit code. That
// the program honors this request at a particular time isn't guaranteed.
//
void kmain_request_program_exit(const int exitCode)
{
    printf("Have been asked to close the program with exit code %d.\n", exitCode);

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
bool kmain_is_program_exiting(void)
{
    return PROGRAM_EXIT_REQUESTED;
}

int main(void)
{
    init_system();

    printf("Entering the main loop...\n");
    {
        // The image buffers we'll render into. Note that the resolution determines
        // the render resolution, which is then upscaled to the resolution of the
        // window.
        image_s<uint8_t, 4> pixelmap(320, 200, 32);
        image_s<double, 1> depthmap(pixelmap.width(), pixelmap.height(), pixelmap.bpp());

        // Load the landscape heightmap and texture map, as well as a polygon object
        // for testing.
        /// TODO: In the future, asset initialization will be handled somewhere other than here.
        image_s<double, 1> terrainHeightmap(QImage("terrain_heightmap.png"));
        image_s<uint8_t, 4> terrainTexture(QImage("terrain_texture.png"));
        std::vector<triangle_s> tris2 = kmesh_mesh_triangles("untitled.vmf");

        for (triangle_s &tri: tris2)
        {
            for (unsigned i = 0; i < 3; i++)
            {
                tri.v[i].pos.x *= 0.01;
                tri.v[i].pos.y *= 0.01;
                tri.v[i].pos.z *= 0.01;
            }
        }

        for (unsigned loops = 0; loops < 3; loops++)
        {
            for (unsigned y = 1; y < terrainHeightmap.height()-1; y++)
            {
                for (unsigned x = 1; x < terrainHeightmap.width()-1; x++)
                {
                    terrainHeightmap.pixel_at(x, y) = terrainHeightmap.interpolated_pixel_at(x + 0.5, y + 0.5);
                    terrainTexture.pixel_at(x, y) = terrainTexture.interpolated_pixel_at(x + 0.5, y + 0.5);
                }
            }
        }

        const auto terrainHeightmapSampler = [&terrainHeightmap](const double x, const double y)->color_s<double, 1>
        {
            return terrainHeightmap.interpolated_pixel_at(x, y);
        };

        const auto terrainTextureSampler = [&terrainTexture](const double x, const double y)->color_s<uint8_t, 4>
        {
            return terrainTexture.interpolated_pixel_at(x, y);
        };

        /// TODO: In the future, camera initialization will be handled somewhere other than here.
        camera_s camera;
        camera.pos = {512, 200, 512};
        camera.orientation = {0.5, -0.3, 0};
        camera.zoom = 1;
        camera.fov = 80;

        while (!PROGRAM_EXIT_REQUESTED)
        {
            static std::deque<uint> fps;
            static unsigned avgFPS = 0;
            static unsigned frameCnt = 0;

            unsigned renderTime, totalTime;
            QElapsedTimer tim;
            tim.start();

            // Prepare for the next frame.
            {
                depthmap.fill({std::numeric_limits<double>::max()});
                pixelmap.fill({0});
                ktext_clear_ui_text_entries();
            }

            // Render the next frame.
            {
                ktext_add_ui_text(std::string("FPS: ") + std::to_string(avgFPS), {10, 20});

                kd_update_input(&camera);

                kr_draw_landscape(terrainHeightmapSampler, terrainTextureSampler, pixelmap, depthmap, camera);
                kr_draw_triangles(tris2, pixelmap, depthmap, camera);

                renderTime = tim.elapsed();
            }

            // Paint the new frame to screen.
            {
                kd_update_display(pixelmap);
                //kd_update_display(depthmap.as<uint8_t, 4>(0.7));

                totalTime = tim.elapsed();
            }

            // Handle any new user input.
            /// TODO: Implement proper input handling. This works for testing, though.
            {
                vector3_s<double> dir = vector3_s<double>{0, 0, 0};

                if (kinput_is_moving_forward()) dir.z = 1;
                if (kinput_is_moving_backward()) dir.z = -1;
                if (kinput_is_moving_right()) dir.x = 1;
                if (kinput_is_moving_left()) dir.x = -1;

                dir *= (matrix44_rotation_s(0, camera.orientation.y, 0) * matrix44_rotation_s(camera.orientation.x, 0, 0));
                dir = dir.normalized()*0.015;
                camera.pos.x += dir.x;
                camera.pos.y += dir.y;
                camera.pos.z += dir.z;

                camera.pos.y = terrainHeightmapSampler(camera.pos.x, camera.pos.z)[0] / 1.0 + 0.8;
            }

            // Statistics.
            {
                const unsigned curFPS = (1000 / (totalTime? totalTime : 1));

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
                    printf("FPS: %d/%d\n", ((renderTime == 0)? 999 : (1000 / renderTime)), ((totalTime == 0)? 999 : (1000 / totalTime)));
                }
                frameCnt++;
            }
        }
    }

    release_system();

    printf("Bye.\n");
    return EXIT_SUCCESS;
}

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

    kd_acquire_display(1440, 900, "\"Vond\" by Tarpeeksi Hyvae Soft");

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
        vond::image<uint8_t, 4> renderBuffer(360, 225, 32);
        vond::image<double, 1> depthMap(renderBuffer.width(), renderBuffer.height(), renderBuffer.bpp());

        /// TODO: In the future, asset initialization will be handled somewhere other than here.
        vond::image<double, 1> landscapeHeightmap(QImage("height.png"));
        vond::image<uint8_t, 4> landscapeTexture(QImage("ground.png"));
        std::vector<vond::triangle> model = kmesh_mesh_triangles("untitled.vmf");

        landscapeHeightmap.bilinear_filter(4);

        const auto landscapeHeightmapSampler = [&landscapeHeightmap](const double x, const double y)->vond::color<double, 1>
        {
            return landscapeHeightmap.bilinear_sample(x, y);
        };

        const auto landscapeTextureSampler = [&landscapeTexture](const double x, const double y)->vond::color<uint8_t, 4>
        {
            if ((x < 0) || (x > landscapeTexture.width()) ||
                (y < 0) || (y > landscapeTexture.height()))
            {
                return {0, 0, 0, 0};
            }

            return landscapeTexture.pixel_at(x, y);
        };

        const auto landscapeSkySampler = [](const vond::vector3<double> &direction)->vond::color<uint8_t, 4>
        {
            // The color at the base of the horizon.
            vond::color<int, 3> horizonColor = {105, 145, 180};

            horizonColor *= 0.95;

            // The amount by which the base horizon color becomes darker towards the zenith.
            const double rayZenithAngle = abs(direction.dot(vond::vector3<double>{0, 1, 0}));
            const int zenithAttenuation = std::min(100, int(100 * rayZenithAngle));

            return {uint8_t(std::min(255, std::max(0, (horizonColor.channel_at(0) - zenithAttenuation)))),
                    uint8_t(std::min(255, std::max(0, (horizonColor.channel_at(1) - zenithAttenuation)))),
                    uint8_t(std::min(255, std::max(0, (horizonColor.channel_at(2) - zenithAttenuation)))),
                    255u};
        };

        /// TODO: In the future, camera initialization will be handled somewhere other than here.
        vond::camera camera;
        camera.pos = {512, 200, 512};
        camera.orientation = {0.5, -0.3, 0};
        camera.zoom = 1;
        camera.fov = 70;

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
                renderBuffer.fill({0, 0, 0, 255});
                depthMap.fill({std::numeric_limits<double>::max()});
                ktext_clear_ui_text_entries();
            }

            // Render the next frame.
            {
                ktext_add_ui_text(std::string("FPS: ") + std::to_string(avgFPS), {10, 20});
                kd_update_input(&camera);

                vond::render_landscape(landscapeHeightmapSampler, landscapeTextureSampler, landscapeSkySampler, renderBuffer, depthMap, camera);
                vond::render_triangles(model, renderBuffer, depthMap, camera);

                renderTime = tim.elapsed();
            }

            // Paint the new frame to screen.
            {
                kd_update_display(renderBuffer);
                //kd_update_display(depthMap.as<uint8_t, 4>(0.7));

                totalTime = tim.elapsed();
            }

            // Handle any new user input.
            /// TODO: Implement proper input handling. This works for testing, though.
            {
                vond::vector3<double> dir = vond::vector3<double>{0, 0, 0};

                if (kinput_is_moving_forward()) dir.z = 1;
                if (kinput_is_moving_backward()) dir.z = -1;
                if (kinput_is_moving_right()) dir.x = 1;
                if (kinput_is_moving_left()) dir.x = -1;

                dir *= (vond::rotation_matrix(0, camera.orientation.y, 0) * vond::rotation_matrix(camera.orientation.x, 0, 0));
                dir = dir.normalized()*6;
                camera.pos.x += dir.x;
                camera.pos.y += dir.y;
                camera.pos.z += dir.z;

                //camera.pos.y = landscapeHeightmapSampler(camera.pos.x, camera.pos.z).channel_at(0) + 18;
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

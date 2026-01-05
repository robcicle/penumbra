#pragma once

#include "ppch.h"

// For use of penumbra applications

#include "core/base.h"
#include "core/assert.h"

#include "core/application.h"
#include "core/layer.h"
#include "core/log.h"

#include "core/time.h"
#include "core/uuid.h"

#include "core/input.h"
#include "core/key_codes.h"
#include "core/mouse_codes.h"
#include "renderer/orthographic_camera_controller.h"
#include "scene/scene_camera.h"

#include "imgui/imgui_layer.h"

#include "events/key_event.h"

#include "project/project.h"

#include "renderer/renderer.h"
#include "renderer/scene_renderer.h"
#include "renderer/render_command.h"

#include "renderer/buffer.h"
#include "renderer/structured_buffer.h"
#include "renderer/shader.h"
#include "renderer/framebuffer.h"
#include "renderer/vertex_array.h"
#include "renderer/material.h"

#include "renderer/texture.h"
#include "renderer/sub_texture2D.h"

#include "renderer/mesh.h"

#include "renderer/orthographic_camera.h"
#include "renderer/primitives.h"

#include "math/math.h"
#include "renderer/editor_camera.h"
#include "utility/platform_utils.h"

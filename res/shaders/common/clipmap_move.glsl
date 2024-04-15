//Trim geometry needs to be mirrored along x/z axis
//during each move. We simulate this with rotations, 
//since we cannot change orientation as we are using face culling.
//This is now done via a switch statement, since using an array of
//rotation matrices was crashing Intel drivers on Windows.
void Rotate(inout vec2 pos, int id)
{
    switch(id)
    {
        case 1:
        {
            pos.xy = vec2(-1,1) * pos.yx;
            return;
        }
        case 2:
        {
            pos.xy = vec2(1,-1) * pos.yx;
            return;
        }
        case 3:
        {
            pos *= -1.0;
            return;
        }
    }
}

vec2 GetClipmapPos(vec2 vertex_pos, vec2 camera_pos, float quad_size, float trim_flag)
{
    vec2 hoffset = camera_pos - mod(camera_pos, quad_size);

    vec2 pos2 = vertex_pos;

    if (trim_flag == 1.0)
    {
        ivec2 id2 = ivec2(hoffset/quad_size);

        id2.x = id2.x % 2;
        id2.y = id2.y % 2;

        int id = id2.x + 2*id2.y;

        Rotate(pos2, id);
        pos2 += quad_size * vec2(1-id2.x, 1-id2.y);
    }

    return pos2 + hoffset;
}

//This overload computes id and id2 for all vertices (not only trim ones) and returns them via inout variables
//It also returns hoffset in the same way and doesn't add it automatically to the result
//This is needed to establish good sampling strategy in the displacement shader
vec2 GetClipmapPos(vec2 vertex_pos, vec2 camera_pos, float quad_size, float trim_flag, inout int id, inout ivec2 id2, inout vec2 hoffset)
{
    hoffset = camera_pos - mod(camera_pos, quad_size);

    vec2 pos2 = vertex_pos;

    id2 = ivec2(hoffset/quad_size);

    id2.x = id2.x % 2;
    id2.y = id2.y % 2;

    id = id2.x + 2*id2.y;

    if (trim_flag == 1.0)
    {
        Rotate(pos2, id);
        pos2 += quad_size * vec2(1-id2.x, 1-id2.y);
    }

    return pos2;
}
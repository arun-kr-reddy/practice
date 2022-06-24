__kernel void
convolution (__global unsigned char *src_buff,
             __global unsigned char *dst_buff)
{
    int x = get_global_id (0);
    int y = get_global_id (1);

    // init
    int ker_half_size = K_SIZE / 2;

    for (int i = 0; i < TILE_SIZE; i++)
    {
        event = async_work_group_copy (
            &dataInLocal[i * dataInLocalWidth],
            &dataIn[(get_group_id (1) * get_local_size (1) - offset + i)
                        * get_global_size (0)
                    + (get_group_id (0) * get_local_size (0)) - offset],
            dataInLocalWidth, event);
    }

    unsigned int sum = 0;
    for (int tile_y = 0; tile_y <= TILE_SIZE; tile_y++)
    {
        for (int tile_x = 0; tile_x <= TILE_SIZE; tile_x++)
        {
            for (int ker_y = -ker_half_size; ker_y <= ker_half_size; ker_y++)
            {
                for (int ker_x = -ker_half_size; ker_x <= ker_half_size;
                     ker_x++)
                {
                    int pos_y = y + ker_y;
                    int pos_x = x + ker_x;
                    if ((pos_x >= 0) && (pos_x < WIDTH) && (pos_y >= 0)
                        && (pos_y < HEIGHT))
                    {
                        sum += src_buff[pos_y * WIDTH + pos_x];
                    }
                }
            }
        }
    }
    dst_buff[y * WIDTH + x] = (unsigned char)(sum / (K_SIZE * K_SIZE));
}

kernel void
using_local (const global float *dataIn, local float *dataInLocal)
{
    event_t event;
    const int dataInLocalWidth = (offset * 2 + get_local_size (0));

    for (int i = 0; i < (offset * 2 + get_local_size (1)); i++)
    {
        event = async_work_group_copy (
            &dataInLocal[i * dataInLocalWidth],
            &dataIn[(get_group_id (1) * get_local_size (1) - offset + i)
                        * get_global_size (0)
                    + (get_group_id (0) * get_local_size (0)) - offset],
            dataInLocalWidth, event);
    }
    do_other_stuff ();             // code that you can execute for free
    wait_group_events (1, &event); // waits until the copy has finished.
    use_data (dataInLocal);
}
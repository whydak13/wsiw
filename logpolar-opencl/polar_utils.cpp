
#include "polar_utils.hpp"


void linspace( float y0, float ymax, float steps, std::vector<float> & vec)
{
    float a = (ymax - y0)/steps;

    vec.clear();
    for (int i = 0; i < steps; i++)
    {
        vec.push_back( a*i + y0 );
    }
}

void create_map(cv::Mat & to_polar_map, int N_s, int N_r, float r_n, float blind, int x_0, int y_0, int src_width)
{
// Function is responsible for creating cordinates map from cartesian space
// to polar space.
// img is input image with size n x m
// for other inputs check main, im to lazy
//  to_polar_map_r n x m array with r index in corespongind N_r x N_s polar
//  image. Inedx, not value!
//  to_polar_map_theta - as above but theta
// polar_img - ready polar image, displaying and debugging mostly
// show_polar_pixels - polar image displayed in cartesian space
    float thet_0 = 0; //beggining of theta ragne
    float thet_max = 2*PI-0.001; //end of theta ragne

    std::vector<float> theta;
    linspace( thet_0, thet_max, N_s+1, theta);

    std::cout << "theta length: " << theta.size() << std::endl;

    std::vector<float> r;
    for( int i = 0; i <= N_r; i++ )
    {
        r.push_back( i*r_n + blind);
    }

    std::cout << "r length: " << r.size() << std::endl;

    to_polar_map = cv::Mat( N_s*N_r, MAX_PIX_COUNT, CV_32S, double(0));

    for(int i = 0; i < N_r; i++)
    {
        for(int j = 0; j < N_s; j++)
        {
            float in_R = r[i];
            float out_R = r[i+1]; //check if doesnt try to access not its memory

            get_polar_pixel( (int32_t *)&to_polar_map.data[(N_s*i+j)*MAX_PIX_COUNT*4], x_0, y_0, in_R, out_R, theta[j], theta[j+1], src_width );

        }
    }

}

void get_polar_pixel(int32_t * coords, int x_0, int y_0, float r_min, float r_max, float thet_min, float thet_max, int src_width )
{
    int x_corners[4] = { r_min*cos(thet_min) + x_0, r_min*cos(thet_max) + x_0, r_max*cos(thet_min) + x_0, r_max*cos(thet_max) + x_0};
    int y_corners[4] = { r_min*sin(thet_min) + y_0, r_min*sin(thet_max) + y_0, r_max*sin(thet_min) + y_0, r_max*sin(thet_max) + y_0};

    int x_max = *std::max_element( x_corners, x_corners+4);
    int x_min = *std::min_element( x_corners, x_corners+4);
    int y_max = *std::max_element( y_corners, y_corners+4);
    int y_min = *std::min_element( y_corners, y_corners+4);


    std::vector<int> x_span;
    std::vector<int> y_span;
    int error_margin = 15;

    for( int i = x_min - error_margin; i <= x_max + error_margin; i++)
    {
        x_span.push_back( i );
    }
    for( int i = y_min - error_margin; i <= y_max + error_margin; i++)
    {
        y_span.push_back( i );
    }

    for( auto&& x_el : x_span)
    {
        for( auto&& y_el : y_span)
        {
            int x = x_el - x_0;
            int y = y_el - y_0;

            float r = sqrt(x*x+y*y);
            float thet = fmod( atan2(y,x) + 2*PI, (2*PI));

            if ( r >= r_min && r <= r_max && thet >= thet_min && thet <= thet_max)
            {
                *coords++ = (int32_t) x_el*src_width + y_el;
            }
        }
    }
}


void to_polar_c(
    uchar* input,
    int32_t* to_polar_map_x,
    int32_t* to_polar_map_y,
//    int * params,
//    __constant int center_h,
//    __constant int center_w,
    uchar* output)
{

    int i; //kolumna
    int j; //wiersz

    int center_w = 128; //from params later
    int center_h = 128; //from params later

    int read_pos;
    int acc;

//    int16_t to_polar_map[128];
    int k;

    for( i = 0; i < 40; i++){
        for( j = 0; j < 40; j++){
            acc = 0;

            for( k = 0; k < 128; k++)
            {
                read_pos = to_polar_map_y[k + (40*j+i)*128];
                if ( read_pos > 0 )
                {
                    read_pos += to_polar_map_x[k + (40*j+i)*128]*256;
                    acc += input[read_pos];
                }
                else
                {
                    break;
                }

            }
            output[40*j+i] = k>0 ? acc/k : 0;
        }
    }

}

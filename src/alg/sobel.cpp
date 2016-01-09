#include <alg/sobel.hpp>
#include <cmn/image.hpp>
#include <cmn/image_utils.hpp>
#include <cmn/log.hpp>
#include <cmn/point.hpp>
#include <math.h>
#include <string.h>

//for tests
#include <adapter/image.hpp>
#include <adapter/filesystem.hpp>
#include <alg/paint.hpp>
#include <alg/spot.hpp>
//#include <alg/diagram.hpp>

namespace alg 
{

#if 0        
static int const mask[3][3]
{
        {-3,     0,      3},
        {-10,    0,      10},
        {-3,     0,      3},        
};
#endif

struct rgb_mask_t
{
        static int const constexpr maskr[3][3] =
        {
                {-3*76,     0,      3*76},
                {-10*76,    0,      10*76},
                {-3*76,     0,      3*76},        
        };
        static int const constexpr maskg[3][3] =
        {
                {-3*150,     0,      3*150},
                {-10*150,    0,      10*150},
                {-3*150,     0,      3*150},        
        };
        static int const constexpr maskb[3][3] =
        {
                {-3*29,     0,      3*29},
                {-10*29,    0,      10*29},
                {-3*29,     0,      3*29},        
        };
};

struct hsv_mask_t
{
        static int const constexpr maskr[3][3] =
        {
                {-3*20,     0,      3*20},
                {-10*20,    0,      10*20},
                {-3*20,     0,      3*20},        
        };
        static int const constexpr maskg[3][3] =
        {
                {-3*110,     0,      3*110},
                {-10*110,    0,      10*110},
                {-3*110,     0,      3*110},        
        };
        static int const constexpr maskb[3][3] =
        {
                {-3*110,     0,      3*110},
                {-10*110,    0,      10*110},
                {-3*110,     0,      3*110},        
        };

};

struct lab_mask_t
{
        static float const constexpr mask[3][3] =
        {
                {-3,     0,      3},
                {-10,    0,      10},
                {-3,     0,      3},        
        };
};

//reason: http://stackoverflow.com/questions/8016780/undefined-reference-to-static-constexpr-char
constexpr const int rgb_mask_t::maskr[3][3];
constexpr const int rgb_mask_t::maskg[3][3];
constexpr const int rgb_mask_t::maskb[3][3];
constexpr const int hsv_mask_t::maskr[3][3];
constexpr const int hsv_mask_t::maskg[3][3];
constexpr const int hsv_mask_t::maskb[3][3];
constexpr const float lab_mask_t::mask[3][3];

template< typename M >          //M stands for Mask, see above 
static void
sobel_template( cmn::image_pt * img_edge, sobel_stat_t * stat, cmn::image_pt const img_src )
{        
        int max_gradi = 0;
        int * const diagram = stat->diagram;
        memset( diagram, 0, 256*sizeof(int) );
        
        int const width = img_src->header.width;
        int const height = img_src->header.height;        
                 
        if( width < 3  || height < 3 )                        
                return;
                
        cmn::image_pt img_dstp = cmn::image_create( width, height, cmn::pitch_default, cmn::format_g );
        cmn::image_t * img_dst = img_dstp.get();
        
        int const lastx = width - 1;
        int const lasty = height - 1;
        
        cmn::color4b_t const * row[3];        
        row[1] = img_src->row<cmn::color4b_t>(0);
        row[2] = img_src->row<cmn::color4b_t>(1);
                        
        for( int y = 1; y < lasty; ++y )
        {
                row[0] = row[1];
                row[1] = row[2];
                row[2] = img_src->row<cmn::color4b_t>(y+1);                
        
                uint8_t * row_dst = img_dst->row<uint8_t>(y);
                
                for( int x = 1; x < lastx; ++x )
                {
                        //compiler optimization heaven...
                        int const x0 = x-1;
                        int const x2 = x+1;
                        
                        typedef cmn::color4b_t c_t;
                        c_t const row00 = row[0][x0];
                        c_t const row01 = row[0][x];
                        c_t const row02 = row[0][x2];
                        c_t const row10 = row[1][x0];
                        c_t const row12 = row[1][x2];
                        c_t const row20 = row[2][x0];
                        c_t const row21 = row[2][x];
                        c_t const row22 = row[2][x2];
                                                
                        int const gx00r = M::maskr[0][0]*row00.r; int const gx00g = M::maskg[0][0]*row00.g; int const gx00b = M::maskb[0][0]*row00.b;
                        int const gx02r = M::maskr[0][2]*row02.r; int const gx02g = M::maskg[0][2]*row02.g; int const gx02b = M::maskb[0][2]*row02.b;
                        int const gx10r = M::maskr[1][0]*row10.r; int const gx10g = M::maskg[1][0]*row10.g; int const gx10b = M::maskb[1][0]*row10.b;
                        int const gx12r = M::maskr[1][2]*row12.r; int const gx12g = M::maskg[1][2]*row12.g; int const gx12b = M::maskb[1][2]*row12.b;
                        int const gx20r = M::maskr[2][0]*row20.r; int const gx20g = M::maskg[2][0]*row20.g; int const gx20b = M::maskb[2][0]*row20.b;
                        int const gx22r = M::maskr[2][2]*row22.r; int const gx22g = M::maskg[2][2]*row22.g; int const gx22b = M::maskb[2][2]*row22.b;

                        int const gy00r = M::maskr[0][0]*row00.r; int const gy00g = M::maskg[0][0]*row00.g; int const gy00b = M::maskb[0][0]*row00.b;
                        int const gy01r = M::maskr[1][0]*row01.r; int const gy01g = M::maskg[1][0]*row01.g; int const gy01b = M::maskb[1][0]*row01.b;
                        int const gy02r = M::maskr[2][0]*row02.r; int const gy02g = M::maskg[2][0]*row02.g; int const gy02b = M::maskb[2][0]*row02.b;
                        int const gy20r = M::maskr[0][2]*row20.r; int const gy20g = M::maskg[0][2]*row20.g; int const gy20b = M::maskb[0][2]*row20.b;
                        int const gy21r = M::maskr[1][2]*row21.r; int const gy21g = M::maskg[1][2]*row21.g; int const gy21b = M::maskb[1][2]*row21.b;
                        int const gy22r = M::maskr[2][2]*row22.r; int const gy22g = M::maskg[2][2]*row22.g; int const gy22b = M::maskb[2][2]*row22.b;

                        int const sumx0r = gx00r + gx02r; int const sumx0g = gx00g + gx02g; int const sumx0b = gx00b + gx02b;
                        int const sumx1r = gx10r + gx12r; int const sumx1g = gx10g + gx12g; int const sumx1b = gx10b + gx12b;  
                        int const sumx2r = gx20r + gx22r; int const sumx2g = gx20g + gx22g; int const sumx2b = gx20b + gx22b;
                        
                        int const sumy0r = gy00r + gy01r; int const sumy0g = gy00g + gy01g; int const sumy0b = gy00b + gy01b;
                        int const sumy1r = gy02r + gy20r; int const sumy1g = gy02g + gy20g; int const sumy1b = gy02b + gy20b;
                        int const sumy2r = gy21r + gy22r; int const sumy2g = gy21g + gy22g; int const sumy2b = gy21b + gy22b;
                        
                        int const sumxr = sumx0r + sumx1r + sumx2r; int const sumxg = sumx0g + sumx1g + sumx2g; int const sumxb = sumx0b + sumx1b + sumx2b;
                        int const sumyr = sumy0r + sumy1r + sumy2r; int const sumyg = sumy0g + sumy1g + sumy2g; int const sumyb = sumy0b + sumy1b + sumy2b;
                        
                        //Normalize: YUV (8bits) + 4Bits for Scharr
                        int const gradxr = sumxr >> (4+8); int const gradxg = sumxg >> (4+8); int const gradxb = sumxb >> (4+8);        
                        int const gradyr = sumyr >> (4+8); int const gradyg = sumyg >> (4+8); int const gradyb = sumyb >> (4+8);    
                                                                        
                        //now calculate distance... invent something, definitely will be the slowest point
                        int const grad = (int)sqrt(gradxr*gradxr + gradyr*gradyr + gradxg*gradxg + gradyg*gradyg + gradxb*gradxb + gradyb*gradyb);
                        row_dst[x] = (uint8_t)grad;             
                        ++diagram[ (uint8_t)grad ];
                        
                        if( grad > max_gradi )
                                max_gradi = grad;
                }
        }

        {
                uint8_t * row_dst0 = img_dst->row<uint8_t>(lasty-1);
                uint8_t * row_dst1 = img_dst->row<uint8_t>(lasty);
                
                for( int x = 0; x <= lastx; ++x )        
                        row_dst1[x] = row_dst0[x];

                row_dst0 = img_dst->row<uint8_t>(0);
                row_dst1 = img_dst->row<uint8_t>(1);
                for( int x = 0; x <= lastx; ++x )        
                        row_dst0[x] = row_dst1[x];

                //vertical access:
                int const prelastx = lastx - 1;
                for( int y = 1; y < lasty; ++y )
                {
                        row_dst0 = img_dst->row<uint8_t>(y);
                        row_dst0[0] = row_dst0[1];
                        row_dst0[lastx] = row_dst0[prelastx];
                }
        }
        
        *img_edge = img_dstp;
        stat->max_grad = max_gradi;
}

template<typename M>    //M here - lab mask...
void sobel_lab(cmn::image_pt * img_edge, sobel_stat_t * stat, cmn::image_pt const img_src)
{
        int max_gradi = 0;
        int * const diagram = stat->diagram;
        memset( diagram, 0, 256*sizeof(int) );
        
        int const width = img_src->header.width;
        int const height = img_src->header.height;        
                 
        if( width < 3  || height < 3 )                        
                return;
                
        cmn::image_pt img_dstp = cmn::image_create( width, height, cmn::pitch_default, cmn::format_g );
        cmn::image_t * img_dst = img_dstp.get();
        
        int const lastx = width - 1;
        int const lasty = height - 1;
        
        cmn::color3f_t const * row[3];        
        row[1] = img_src->row<cmn::color3f_t>(0);
        row[2] = img_src->row<cmn::color3f_t>(1);
                        
        for( int y = 1; y < lasty; ++y )
        {
                row[0] = row[1];
                row[1] = row[2];
                row[2] = img_src->row<cmn::color3f_t>(y+1);                
        
                uint8_t * row_dst = img_dst->row<uint8_t>(y);
                
                for( int x = 1; x < lastx; ++x )
                {
                        //compiler optimization heaven...
                        int const x0 = x-1;
                        int const x2 = x+1;
                        
                        typedef cmn::color3f_t c_t;
                        c_t const row00 = row[0][x0];
                        c_t const row01 = row[0][x];
                        c_t const row02 = row[0][x2];
                        c_t const row10 = row[1][x0];
                        c_t const row12 = row[1][x2];
                        c_t const row20 = row[2][x0];
                        c_t const row21 = row[2][x];
                        c_t const row22 = row[2][x2];
                                                
                        float const gx00r = M::mask[0][0]*row00.r; float const gx00g = M::mask[0][0]*row00.g; float const gx00b = M::mask[0][0]*row00.b;
                        float const gx02r = M::mask[0][2]*row02.r; float const gx02g = M::mask[0][2]*row02.g; float const gx02b = M::mask[0][2]*row02.b;
                        float const gx10r = M::mask[1][0]*row10.r; float const gx10g = M::mask[1][0]*row10.g; float const gx10b = M::mask[1][0]*row10.b;
                        float const gx12r = M::mask[1][2]*row12.r; float const gx12g = M::mask[1][2]*row12.g; float const gx12b = M::mask[1][2]*row12.b;
                        float const gx20r = M::mask[2][0]*row20.r; float const gx20g = M::mask[2][0]*row20.g; float const gx20b = M::mask[2][0]*row20.b;
                        float const gx22r = M::mask[2][2]*row22.r; float const gx22g = M::mask[2][2]*row22.g; float const gx22b = M::mask[2][2]*row22.b;

                        float const gy00r = M::mask[0][0]*row00.r; float const gy00g = M::mask[0][0]*row00.g; float const gy00b = M::mask[0][0]*row00.b;
                        float const gy01r = M::mask[1][0]*row01.r; float const gy01g = M::mask[1][0]*row01.g; float const gy01b = M::mask[1][0]*row01.b;
                        float const gy02r = M::mask[2][0]*row02.r; float const gy02g = M::mask[2][0]*row02.g; float const gy02b = M::mask[2][0]*row02.b;
                        float const gy20r = M::mask[0][2]*row20.r; float const gy20g = M::mask[0][2]*row20.g; float const gy20b = M::mask[0][2]*row20.b;
                        float const gy21r = M::mask[1][2]*row21.r; float const gy21g = M::mask[1][2]*row21.g; float const gy21b = M::mask[1][2]*row21.b;
                        float const gy22r = M::mask[2][2]*row22.r; float const gy22g = M::mask[2][2]*row22.g; float const gy22b = M::mask[2][2]*row22.b;

                        float const sumx0r = gx00r + gx02r; float const sumx0g = gx00g + gx02g; float const sumx0b = gx00b + gx02b;
                        float const sumx1r = gx10r + gx12r; float const sumx1g = gx10g + gx12g; float const sumx1b = gx10b + gx12b;  
                        float const sumx2r = gx20r + gx22r; float const sumx2g = gx20g + gx22g; float const sumx2b = gx20b + gx22b;
                        
                        float const sumy0r = gy00r + gy01r; float const sumy0g = gy00g + gy01g; float const sumy0b = gy00b + gy01b;
                        float const sumy1r = gy02r + gy20r; float const sumy1g = gy02g + gy20g; float const sumy1b = gy02b + gy20b;
                        float const sumy2r = gy21r + gy22r; float const sumy2g = gy21g + gy22g; float const sumy2b = gy21b + gy22b;
                        
                        float const sumxr = sumx0r + sumx1r + sumx2r; float const sumxg = sumx0g + sumx1g + sumx2g; float const sumxb = sumx0b + sumx1b + sumx2b;
                        float const sumyr = sumy0r + sumy1r + sumy2r; float const sumyg = sumy0g + sumy1g + sumy2g; float const sumyb = sumy0b + sumy1b + sumy2b;

                        //normalize
                        //16 should be here instead of 8, but practically I see that max Lab distance I have on images is about 100,
                        //so I think scale x2 is fully ok
                        float const gradxr = sumxr / 8.f; float const gradxg = sumxg / 8.f; float const gradxb = sumxb / 8.f;
                        float const gradyr = sumyr / 8.f; float const gradyg = sumyg / 8.f; float const gradyb = sumyb / 8.f;
                        
                        //now calculate distance... invent something, definitely will be the slowest point
                        int grad = (int)sqrt(gradxr*gradxr + gradyr*gradyr + gradxg*gradxg + gradyg*gradyg + gradxb*gradxb + gradyb*gradyb);
                        
                        if( grad > max_gradi )
                                max_gradi = grad;
                        
                        if( grad > 255.f ) grad = 255.f;        //clamping
                        row_dst[x] = (uint8_t)grad;             
                        ++diagram[ (uint8_t)grad ];
                        
                }
        }

        {
                uint8_t * row_dst0 = img_dst->row<uint8_t>(lasty-1);
                uint8_t * row_dst1 = img_dst->row<uint8_t>(lasty);
                
                for( int x = 0; x <= lastx; ++x )        
                        row_dst1[x] = row_dst0[x];

                row_dst0 = img_dst->row<uint8_t>(0);
                row_dst1 = img_dst->row<uint8_t>(1);
                for( int x = 0; x <= lastx; ++x )        
                        row_dst0[x] = row_dst1[x];

                //vertical access:
                int const prelastx = lastx - 1;
                for( int y = 1; y < lasty; ++y )
                {
                        row_dst0 = img_dst->row<uint8_t>(y);
                        row_dst0[0] = row_dst0[1];
                        row_dst0[lastx] = row_dst0[prelastx];
                }
        }
        
        *img_edge = img_dstp;
        stat->max_grad = max_gradi;        
}

void sobel( cmn::image_pt * img_edge, sobel_stat_t * stat, cmn::image_pt const img_src )
{
        switch( img_src->header.format )
        {
                case cmn::format_rgba:
                        sobel_template<rgb_mask_t>( img_edge, stat, img_src );
                        break;
                case cmn::format_hsva:
                        sobel_template<hsv_mask_t>( img_edge, stat, img_src );
                        break;
                case cmn::format_lab_f32:
                        sobel_lab<lab_mask_t>( img_edge, stat, img_src );
                        break;
                default:
                        cmn::log_and_throw("alg/sobel.cpp:sobel - format %d is not supported", img_src->header.format );
        }
}

#if 0   //works but results are not better than with sobel 3x3

static int mask_f = 1;

//I tried severel different masks... anyway it doesn't become better :(
//sum 30
static int const maskg[5][5] =
{
        {2,      1,      0,      -1,     -2},
        {4,      2,      0,      -2,     -4},
        {8,      4,      0,      -4,     -8},
        {4,      2,      0,      -2,     -4},
        {2,      1,      0,      -1,     -2},
};

static void sobel5( cmn::image_pt * img_edge, sobel_stat_t * stat, cmn::image_pt const img_src )
{
        int max_gradi = 0;
        int * const diagram = stat->diagram;
        memset( diagram, 0, 256*sizeof(int) );
        
        int const width = img_src->header.width;
        int const height = img_src->header.height;        
                 
        if( width < 5  || height < 5 )
                return;
                
        cmn::image_pt img_dstp = cmn::image_create( width, height, cmn::pitch_default, cmn::format_g );
        cmn::image_t * img_dst = img_dstp.get();
        
        int const lastx = width - 2;
        int const lasty = height - 2;
        
        cmn::color4b_t const * row[5];        
        row[1] = img_src->row<cmn::color4b_t>(0);
        row[2] = img_src->row<cmn::color4b_t>(1);
        row[3] = img_src->row<cmn::color4b_t>(2);
        row[4] = img_src->row<cmn::color4b_t>(3);        
                        
        for( int y = 2; y < lasty; ++y )
        {
                //shifting...
                row[0] = row[1];
                row[1] = row[2];
                row[2] = row[3];
                row[3] = row[4];
                row[4] = img_src->row<cmn::color4b_t>(y+2);                
        
                uint8_t * row_dst = img_dst->row<uint8_t>(y);
                
                for( int x = 2; x < lastx; ++x )
                {
                        int sumx = 0, sumy = 0;
                        for( int8_t my = 0; my < 5; ++my )
                        {                        
                                for( int8_t mx = 0; mx < 5; ++mx )
                                {
                                    sumx += row[my][x + mx-2].g*maskg[my][mx];
                                    sumy += row[my][x + mx-2].g*maskg[mx][my];                                    
                                }
                        }
                        
                        int grad = (int)(sqrt( sumx*sumx + sumy*sumy ));
                        grad = grad / 30;
                        row_dst[x] = grad;
                        
                        if( grad > max_gradi )
                                max_gradi = grad;
                }
        }
        
        stat->max_grad = max_gradi;
        *img_edge = img_dstp;
}
#endif


static void sobel_test_do_rest( cmn::image_pt const & img, std::string const & name_base )
{
        std::string dir_dst = adapter::fs_prefs_dir();
        cmn::image_pt img_sobel;
        sobel_stat_t stat;        
        
        sobel(&img_sobel, &stat, img);                
      
        cmn::image_pt img_sobel_color = cmn::image_rgba_from_g8(img_sobel);        
        printf("%s: max_grad: %d\n", name_base.c_str(), stat.max_grad);                
        if( stat.max_grad > 255 ) stat.max_grad = 255;        
        
        adapter::image_save_to_png( img_sobel_color, (dir_dst + name_base +"_sobel.png").c_str() );
        
        //alg::diagram_make_integral( stat.diagram, stat.max_grad );
        //uint8_t cutting_point = alg::diagram_find_cutting_point( stat.diagram, stat.max_grad );
                
        cmn::image_pt img_colored_g16 = alg::image_paint_with_hint( img_sobel, img, 0.07f );
        //cmn::image_pt img_colored_g16 = alg::image_paint_with_hint2( img_sobel, img, 0.04f );
        
        cmn::image_pt img_colored_rgba = alg::spot_color(img_colored_g16);
        
        adapter::image_save_to_png( img_colored_rgba, (dir_dst + name_base +"_sobel_colored.png").c_str() );

}

void sobel_test()
{
        std::string dir_src = adapter::fs_resource_dir() + "pictures/faces_png/";
        std::string dir_dst = adapter::fs_prefs_dir();
        std::string dir_objects = dir_dst + "objects/";                
        adapter::fs_make_dir( dir_objects );                

        std::vector<adapter::fs_file_info_t> files;
        //adapter::fs_dir_contents( &files, dir_src );
        
        adapter::fs_file_info_t fi1;
        fi1.name = "post-1178333-1266493347.png";
        files.push_back(fi1);
        
        for( size_t i = 0; i < files.size(); ++i )
        {        
                adapter::fs_file_info_t const & fi = files[i];
                
                cmn::image_pt img = adapter::image_create_from_png( (dir_src + fi.name).c_str() );        
                cmn::image_pt img_lab = cmn::image_lab_from_rgba( img );
                
                std::string base_name = "objects/" + adapter::fs_name_ext( fi.name ).first;
                
                //sobel_test_do_rest(img, base_name);
                sobel_test_do_rest(img_lab, base_name+"_lab");   
        }                
}

}
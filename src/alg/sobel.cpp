#include <alg/sobel.hpp>
#include <cmn/image.hpp>
#include <cmn/image_utils.hpp>
#include <cmn/point.hpp>
#include <math.h>

//for tests
#include <adapter/image.hpp>
#include <adapter/filesystem.hpp>
#include <alg/paint.hpp>
#include <alg/seg_object.hpp>

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

static int const maskr[3][3]
{
        {-3*76,     0,      3*76},
        {-10*76,    0,      10*76},
        {-3*76,     0,      3*76},        
};
static int const maskg[3][3]
{
        {-3*150,     0,      3*150},
        {-10*150,    0,      10*150},
        {-3*150,     0,      3*150},        
};
static int const maskb[3][3]
{
        {-3*29,     0,      3*29},
        {-10*29,    0,      10*29},
        {-3*29,     0,      3*29},        
};


void
sobel( cmn::image_pt * img_edge, int * max_grad, cmn::image_pt const img_src )
{
        int max_gradi = 0;
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
                                                
                        int const gx00r = maskr[0][0]*row00.r; int const gx00g = maskg[0][0]*row00.g; int const gx00b = maskb[0][0]*row00.b;
                        int const gx02r = maskr[0][2]*row02.r; int const gx02g = maskg[0][2]*row02.g; int const gx02b = maskb[0][2]*row02.b;
                        int const gx10r = maskr[1][0]*row10.r; int const gx10g = maskg[1][0]*row10.g; int const gx10b = maskb[1][0]*row10.b;
                        int const gx12r = maskr[1][2]*row12.r; int const gx12g = maskg[1][2]*row12.g; int const gx12b = maskb[1][2]*row12.b;
                        int const gx20r = maskr[2][0]*row20.r; int const gx20g = maskg[2][0]*row20.g; int const gx20b = maskb[2][0]*row20.b;
                        int const gx22r = maskr[2][2]*row22.r; int const gx22g = maskg[2][2]*row22.g; int const gx22b = maskb[2][2]*row22.b;

                        int const gy00r = maskr[0][0]*row00.r; int const gy00g = maskg[0][0]*row00.g; int const gy00b = maskb[0][0]*row00.b;
                        int const gy01r = maskr[1][0]*row01.r; int const gy01g = maskg[1][0]*row01.g; int const gy01b = maskb[1][0]*row01.b;
                        int const gy02r = maskr[2][0]*row02.r; int const gy02g = maskg[2][0]*row02.g; int const gy02b = maskb[2][0]*row02.b;
                        int const gy20r = maskr[0][2]*row20.r; int const gy20g = maskg[0][2]*row20.g; int const gy20b = maskb[0][2]*row20.b;
                        int const gy21r = maskr[1][2]*row21.r; int const gy21g = maskg[1][2]*row21.g; int const gy21b = maskb[1][2]*row21.b;
                        int const gy22r = maskr[2][2]*row22.r; int const gy22g = maskg[2][2]*row22.g; int const gy22b = maskb[2][2]*row22.b;

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
                        
                        if( grad > max_gradi )
                                max_gradi = grad;
                }
        }

        {
                uint8_t * row_dst0 = img_dst->row<uint8_t>(lasty-1);
                uint8_t * row_dst1 = img_dst->row<uint8_t>(lasty);
                
                for( int x = 0; x <= lastx; ++x )        
                        row_dst1[x] = row_dst0[x];

                row_dst0 = img_src->row<uint8_t>(0);
                row_dst1 = img_src->row<uint8_t>(1);
                for( int x = 0; x <= lastx; ++x )        
                        row_dst0[x] = row_dst1[x];

                //vertical access:
                int const prelastx = lastx - 1;
                for( int y = 1; y < lasty; ++y )
                {
                        row_dst0 = img_src->row<uint8_t>(y);
                        row_dst0[0] = row_dst0[1];
                        row_dst0[lastx] = row_dst0[prelastx];
                }
        }
        
        *img_edge = img_dstp;
        if( max_grad ) *max_grad = max_gradi;
}

void sobel_test()
{
        std::string dir_src = adapter::fs_resource_dir() + "pictures/faces_png/";
        std::string dir_dst = adapter::fs_prefs_dir();
        std::string dir_objects = dir_dst + "objects/";                
        adapter::fs_make_dir( dir_objects );                
        cmn::image_pt img = adapter::image_create_from_png( (dir_src + "c.png").c_str() );        
        
        cmn::image_pt img_sobel;
        int max_grad;
        sobel(&img_sobel, &max_grad, img);
        cmn::image_pt img_sobel_color = cmn::image_rgba_from_g8(img_sobel);        
        printf("max_grad: %d\n", max_grad);                
        adapter::image_save_to_png( img_sobel_color, (dir_dst + "a_sobel.png").c_str() );
        
        cmn::image_pt img_bw = cmn::image_bw_from_g8(img_sobel, max_grad/5);
        cmn::image_pt img_colored_g16 = alg::image_paint(img_bw);
        cmn::image_pt img_colored_rgba = alg::seg_color(img_colored_g16);
        
        adapter::image_save_to_png( img_colored_rgba, (dir_dst + "a_sobel_colored.png").c_str() );
        
}

}
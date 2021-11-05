//
// Created by camilo on 2021-05-10 04:07 BRT <3ThomasBS_
//


inline void copy(rectangle_i32 * prectangleDst, const cairo_rectangle_int_t * prectangleSrc)
{

   prectangleDst->left = prectangleSrc->x;
   prectangleDst->top = prectangleSrc->y;
   prectangleDst->right = prectangleSrc->x + prectangleSrc->width;
   prectangleDst->bottom = prectangleSrc->y + prectangleSrc->height;

}


inline void copy(cairo_rectangle_int_t * prectangleDst, const rectangle_i32 * prectangleSrc)
{

   prectangleDst->x = prectangleSrc->left;
   prectangleDst->y = prectangleSrc->top;
   prectangleDst->width = prectangleSrc->right - prectangleSrc->left;
   prectangleDst->height = prectangleSrc->bottom - prectangleSrc->top;

}




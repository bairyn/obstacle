gfx/misc/dp_none
{
  {
    map gfx/misc/dp_none.jpg
    blendfunc gl_one gl_one
  }
}

gfx/misc/dp_alien
{
  {
    map gfx/misc/dp_alien.jpg
    blendfunc gl_one gl_one
  }
}

gfx/misc/dp_human
{
  {
    map gfx/misc/dp_human.jpg
    blendfunc gl_one gl_one
  }
}

gfx/misc/dp_bar
{
  cull none
  entityMergable
  {
    map gfx/misc/dp_none.jpg
    blendfunc gl_one gl_one
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/misc/dp_bar_h
{
  cull none
  entityMergable
  {
    map gfx/misc/dp_human.jpg
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/misc/dp_bar_a
{
  cull none
  entityMergable
  {
    map gfx/misc/dp_alien.jpg
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/misc/dp_icon_a
{
  {
    map gfx/misc/dp_icon_a.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/misc/dp_icon_b
{
  {
    map gfx/misc/dp_icon_b.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/misc/dp_icon_c
{
  {
    map gfx/misc/dp_icon_c.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/misc/dp_icon_d
{
  {
    map gfx/misc/dp_icon_d.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen vertex
    alphaGen vertex
  }
}

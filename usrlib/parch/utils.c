#include "../header/utils.h"
#include "../header/syscall.h"

void tb_memcpy(u8* src, u8* dst, u64 len) {
    while (len --> 0) {
        *(dst++) = *(src++);
    }
}

void tb_strcpy(char* src, char* dst) {
    while (*src) {
        *(dst++) = *(src++);
    }
    *dst = 0;
}
    
u64 tb_strlen(char* str) {
    char* p = str;
    while(*p) p++;
    return p - str;
}

void tb_memset(u8* buf, u8 byte, u64 len) {
    while (len --> 0) {
        *(buf++) = 0;
    }
}



// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.
// copied shamlessly from [xv6-riscv](git@github.com:mit-pdos/xv6-riscv.git)
// seems to be a linked list allocator, slightly modified to adopt api.
// MIT Licese for this code segment:
// ==========================================================================
// The xv6 software is:
// 
// Copyright (c) 2006-2019 Frans Kaashoek, Robert Morris, Russ Cox,
//                         Massachusetts Institute of Technology
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


typedef long Align;

union header {
  struct {
    union header *ptr;
    u64 size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
tb_free(void *ap)
{
  Header *bp, *p;

  bp = (Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

static Header*
morecore(u32 nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = tb_sbrk(nu * sizeof(Header));
  if(p == (char*)-1)
    return 0;
  hp = (Header*)p;
  hp->s.size = nu;
  tb_free((void*)(hp + 1));
  return freep;
}

void*
tb_malloc(u64 nbytes)
{
  Header *p, *prevp;
  u64 nunits;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}
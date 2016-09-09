#include "edosu-circle.h"

struct vertex_data const edosu_circle_data[] = {
    { { 0.000000 ,-1.000000  } },
    { { -0.195090, -0.980785 }  },
    { { -0.382683, -0.923880 }  },
    { { -0.555570, -0.831470 }  },
    { { -0.707107, -0.707107 }  },
    { { -0.831470, -0.555570 }  },
    { { -0.923880, -0.382683 }  },
    { { -0.980785, -0.195090 }  },
    { { -1.000000, -0.000000 }  },
    { { -0.980785, 0.195090  } },
    { { -0.923880, 0.382683  } },
    { { -0.831470, 0.555570  } },
    { { -0.707107, 0.707107  } },
    { { -0.555570, 0.831470  } },
    { { -0.382683, 0.923880  } },
    { { -0.195090, 0.980785  } },
    { { 0.000000 ,1.000000 } } ,
    { { 0.195091 ,0.980785 } } ,
    { { 0.382684 ,0.923879 } } ,
    { { 0.555571 ,0.831469 } } ,
    { { 0.707107 ,0.707106 } } ,
    { { 0.831470 ,0.555570 } } ,
    { { 0.923880 ,0.382683 } } ,
    { { 0.980785 ,0.195089 } } ,
    { { 1.000000 ,-0.000001  } },
    { { 0.980785 ,-0.195091  } },
    { { 0.923879 ,-0.382684  } },
    { { 0.831469 ,-0.555571  } },
    { { 0.707106 ,-0.707108  } },
    { { 0.555569 ,-0.831470  } },
    { { 0.382682 ,-0.923880  } },
    { { 0.195089 ,-0.980786  } },
    { { 0.000000 ,0.000000 } } ,
};
size_t edosu_circle_data_size = sizeof(edosu_circle_data);

uint8_t const edosu_circle_index[] = {
    19, 18, 32,
    18, 17, 32,
    16, 32, 17,
    15, 32, 16,
    14, 32, 15,
    13, 32, 14,
    12, 32, 13,
    11, 32, 12,
    10, 32, 11,
    9, 32, 10,
    8, 32, 9,
    7, 32, 8,
    6, 32, 7,
    5, 32, 6,
    4, 32, 5,
    3, 32, 4,
    2, 32, 3,
    1, 32, 2,
    0, 32, 1,
    31, 32, 0,
    30, 32, 31,
    29, 32, 30,
    28, 32, 29,
    27, 32, 28,
    26, 32, 27,
    25, 32, 26,
    24, 32, 25,
    23, 32, 24,
    22, 32, 23,
    21, 32, 22,
    20, 32, 21,
    32, 20, 19,
};
size_t edosu_circle_index_size = sizeof(edosu_circle_index);
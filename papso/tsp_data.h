#pragma once
#include <array>

using coord = std::pair<double, double>;
struct tsp_data
{
	static constexpr coord berlin52[] = {
		{ 565.0, 575.0 },
		{ 25.0, 185.0 },
		{ 345.0, 750.0 },
		{ 945.0, 685.0 },
		{ 845.0, 655.0 },
		{ 880.0, 660.0 },
		{ 25.0, 230.0 },
		{ 525.0, 1000.0 },
		{ 580.0, 1175.0 },
		{ 650.0, 1130.0 },
		{ 1605.0, 620.0 },
		{ 1220.0, 580.0 },
		{ 1465.0, 200.0 },
		{ 1530.0, 5.0 },
		{ 845.0, 680.0 },
		{ 725.0, 370.0 },
		{ 145.0, 665.0 },
		{ 415.0, 635.0 },
		{ 510.0, 875.0 },
		{ 560.0, 365.0 },
		{ 300.0, 465.0 },
		{ 520.0, 585.0 },
		{ 480.0, 415.0 },
		{ 835.0, 625.0 },
		{ 975.0, 580.0 },
		{ 1215.0, 245.0 },
		{ 1320.0, 315.0 },
		{ 1250.0, 400.0 },
		{ 660.0, 180.0 },
		{ 410.0, 250.0 },
		{ 420.0, 555.0 },
		{ 575.0, 665.0 },
		{ 1150.0, 1160.0 },
		{ 700.0, 580.0 },
		{ 685.0, 595.0 },
		{ 685.0, 610.0 },
		{ 770.0, 610.0 },
		{ 795.0, 645.0 },
		{ 720.0, 635.0 },
		{ 760.0, 650.0 },
		{ 475.0, 960.0 },
		{ 95.0, 260.0 },
		{ 875.0, 920.0 },
		{ 700.0, 500.0 },
		{ 555.0, 815.0 },
		{ 830.0, 485.0 },
		{ 1170.0, 65.0 },
		{ 830.0, 610.0 },
		{ 605.0, 625.0 },
		{ 595.0, 360.0 },
		{ 1340.0, 725.0 },
		{ 1740.0, 245.0 }
	};
	static constexpr coord pr76[] = {
		{ 3600, 2300 },
		{ 3100, 3300 },
		{ 4700, 5750 },
		{ 5400, 5750 },
		{ 5608, 7103 },
		{ 4493, 7102 },
		{ 3600, 6950 },
		{ 3100, 7250 },
		{ 4700, 8450 },
		{ 5400, 8450 },
		{ 5610, 10053 },
		{ 4492, 10052 },
		{ 3600, 10800 },
		{ 3100, 10950 },
		{ 4700, 11650 },
		{ 5400, 11650 },
		{ 6650, 10800 },
		{ 7300, 10950 },
		{ 7300, 7250 },
		{ 6650, 6950 },
		{ 7300, 3300 },
		{ 6650, 2300 },
		{ 5400, 1600 },
		{ 8350, 2300 },
		{ 7850, 3300 },
		{ 9450, 5750 },
		{ 10150, 5750 },
		{ 10358, 7103 },
		{ 9243, 7102 },
		{ 8350, 6950 },
		{ 7850, 7250 },
		{ 9450, 8450 },
		{ 10150, 8450 },
		{ 10360, 10053 },
		{ 9242, 10052 },
		{ 8350, 10800 },
		{ 7850, 10950 },
		{ 9450, 11650 },
		{ 10150, 11650 },
		{ 11400, 10800 },
		{ 12050, 10950 },
		{ 12050, 7250 },
		{ 11400, 6950 },
		{ 12050, 3300 },
		{ 11400, 2300 },
		{ 10150, 1600 },
		{ 13100, 2300 },
		{ 12600, 3300 },
		{ 14200, 5750 },
		{ 14900, 5750 },
		{ 15108, 7103 },
		{ 13993, 7102 },
		{ 13100, 6950 },
		{ 12600, 7250 },
		{ 14200, 8450 },
		{ 14900, 8450 },
		{ 15110, 10053 },
		{ 13992, 10052 },
		{ 13100, 10800 },
		{ 12600, 10950 },
		{ 14200, 11650 },
		{ 14900, 11650 },
		{ 16150, 10800 },
		{ 16800, 10950 },
		{ 16800, 7250 },
		{ 16150, 6950 },
		{ 16800, 3300 },
		{ 16150, 2300 },
		{ 14900, 1600 },
		{ 19800, 800 },
		{ 19800, 10000 },
		{ 19800, 11900 },
		{ 19800, 12200 },
		{ 200, 12200 },
		{ 200, 1100 },
		{ 200, 800 }
	};
	static constexpr coord gr96[] = {
		{ 14.55, -23.31 },
		{ 28.06, -15.24 },
		{ 32.38, -16.54 },
		{ 31.38, -8.00 },
		{ 33.39, -7.35 },
		{ 34.02, -6.51 },
		{ 34.05, -4.57 },
		{ 35.48, -5.45 },
		{ 35.43, -0.43 },
		{ 36.47, 3.03 },
		{ 22.56, 5.30 },
		{ 36.22, 6.37 },
		{ 36.48, 10.11 },
		{ 34.44, 10.46 },
		{ 32.54, 13.11 },
		{ 32.07, 20.04 },
		{ 31.12, 29.54 },
		{ 31.16, 32.18 },
		{ 29.58, 32.33 },
		{ 30.03, 31.15 },
		{ 24.05, 32.53 },
		{ 19.37, 37.14 },
		{ 15.36, 32.32 },
		{ 13.11, 30.13 },
		{ 13.38, 25.21 },
		{ 15.20, 38.53 },
		{ 9.00, 38.50 },
		{ 11.36, 43.09 },
		{ 18.06, -15.57 },
		{ 14.40, -17.26 },
		{ 13.28, -16.39 },
		{ 11.51, -15.35 },
		{ 16.46, -3.01 },
		{ 12.39, -8.00 },
		{ 10.23, -9.18 },
		{ 9.31, -13.43 },
		{ 8.30, -13.15 },
		{ 6.18, -10.47 },
		{ 5.19, -4.02 },
		{ 6.41, -1.35 },
		{ 5.33, -0.13 },
		{ 6.08, 1.13 },
		{ 6.29, 2.37 },
		{ 12.22, -1.31 },
		{ 13.31, 2.07 },
		{ 12.00, 8.30 },
		{ 11.51, 13.10 },
		{ 12.07, 15.03 },
		{ 6.27, 3.24 },
		{ 6.27, 7.27 },
		{ 0.20, 6.44 },
		{ 3.45, 8.47 },
		{ 3.52, 11.31 },
		{ 4.22, 18.35 },
		{ 0.23, 9.27 },
		{ -4.16, 15.17 },
		{ -4.18, 15.18 },
		{ 0.04, 18.16 },
		{ -5.54, 22.25 },
		{ 0.30, 25.12 },
		{ -3.23, 29.22 },
		{ -1.57, 30.04 },
		{ 0.19, 32.25 },
		{ -1.17, 36.49 },
		{ 2.01, 45.20 },
		{ -4.03, 39.40 },
		{ -6.10, 39.11 },
		{ -6.48, 39.17 },
		{ -8.48, 13.14 },
		{ -12.44, 15.47 },
		{ -11.40, 27.28 },
		{ -12.49, 28.13 },
		{ -15.25, 28.17 },
		{ -20.09, 28.36 },
		{ -17.50, 31.03 },
		{ -15.47, 35.00 },
		{ -19.49, 34.52 },
		{ -25.58, 32.35 },
		{ -15.57, -5.42 },
		{ -37.15, -12.30 },
		{ -22.59, 14.31 },
		{ -22.34, 17.06 },
		{ -26.38, 15.10 },
		{ -24.45, 25.55 },
		{ -25.45, 28.10 },
		{ -26.15, 28.00 },
		{ -29.12, 26.07 },
		{ -29.55, 30.56 },
		{ -33.00, 27.55 },
		{ -33.58, 25.40 },
		{ -33.55, 18.22 },
		{ -23.21, 43.40 },
		{ -18.55, 47.31 },
		{ -12.16, 49.17 },
		{ -20.10, 57.30 },
		{ -4.38, 55.27 }
	};
	static constexpr coord kroA100[] = {
		{ 1380, 939 },
		{ 2848, 96 },
		{ 3510, 1671 },
		{ 457, 334 },
		{ 3888, 666 },
		{ 984, 965 },
		{ 2721, 1482 },
		{ 1286, 525 },
		{ 2716, 1432 },
		{ 738, 1325 },
		{ 1251, 1832 },
		{ 2728, 1698 },
		{ 3815, 169 },
		{ 3683, 1533 },
		{ 1247, 1945 },
		{ 123, 862 },
		{ 1234, 1946 },
		{ 252, 1240 },
		{ 611, 673 },
		{ 2576, 1676 },
		{ 928, 1700 },
		{ 53, 857 },
		{ 1807, 1711 },
		{ 274, 1420 },
		{ 2574, 946 },
		{ 178, 24 },
		{ 2678, 1825 },
		{ 1795, 962 },
		{ 3384, 1498 },
		{ 3520, 1079 },
		{ 1256, 61 },
		{ 1424, 1728 },
		{ 3913, 192 },
		{ 3085, 1528 },
		{ 2573, 1969 },
		{ 463, 1670 },
		{ 3875, 598 },
		{ 298, 1513 },
		{ 3479, 821 },
		{ 2542, 236 },
		{ 3955, 1743 },
		{ 1323, 280 },
		{ 3447, 1830 },
		{ 2936, 337 },
		{ 1621, 1830 },
		{ 3373, 1646 },
		{ 1393, 1368 },
		{ 3874, 1318 },
		{ 938, 955 },
		{ 3022, 474 },
		{ 2482, 1183 },
		{ 3854, 923 },
		{ 376, 825 },
		{ 2519, 135 },
		{ 2945, 1622 },
		{ 953, 268 },
		{ 2628, 1479 },
		{ 2097, 981 },
		{ 890, 1846 },
		{ 2139, 1806 },
		{ 2421, 1007 },
		{ 2290, 1810 },
		{ 1115, 1052 },
		{ 2588, 302 },
		{ 327, 265 },
		{ 241, 341 },
		{ 1917, 687 },
		{ 2991, 792 },
		{ 2573, 599 },
		{ 19, 674 },
		{ 3911, 1673 },
		{ 872, 1559 },
		{ 2863, 558 },
		{ 929, 1766 },
		{ 839, 620 },
		{ 3893, 102 },
		{ 2178, 1619 },
		{ 3822, 899 },
		{ 378, 1048 },
		{ 1178, 100 },
		{ 2599, 901 },
		{ 3416, 143 },
		{ 2961, 1605 },
		{ 611, 1384 },
		{ 3113, 885 },
		{ 2597, 1830 },
		{ 2586, 1286 },
		{ 161, 906 },
		{ 1429, 134 },
		{ 742, 1025 },
		{ 1625, 1651 },
		{ 1187, 706 },
		{ 1787, 1009 },
		{ 22, 987 },
		{ 3640, 43 },
		{ 3756, 882 },
		{ 776, 392 },
		{ 1724, 1642 },
		{ 198, 1810 },
		{ 3950, 1558 }

	};
	static constexpr coord lin105[] = {
		{ 63, 71 },
		{ 94, 71 },
		{ 142, 370 },
		{ 173, 1276 },
		{ 205, 1213 },
		{ 213, 69 },
		{ 244, 69 },
		{ 276, 630 },
		{ 283, 732 },
		{ 362, 69 },
		{ 394, 69 },
		{ 449, 370 },
		{ 480, 1276 },
		{ 512, 1213 },
		{ 528, 157 },
		{ 583, 630 },
		{ 591, 732 },
		{ 638, 654 },
		{ 638, 496 },
		{ 638, 314 },
		{ 638, 142 },
		{ 669, 142 },
		{ 677, 315 },
		{ 677, 496 },
		{ 677, 654 },
		{ 709, 654 },
		{ 709, 496 },
		{ 709, 315 },
		{ 701, 142 },
		{ 764, 220 },
		{ 811, 189 },
		{ 843, 173 },
		{ 858, 370 },
		{ 890, 1276 },
		{ 921, 1213 },
		{ 992, 630 },
		{ 1000, 732 },
		{ 1197, 1276 },
		{ 1228, 1213 },
		{ 1276, 205 },
		{ 1299, 630 },
		{ 1307, 732 },
		{ 1362, 654 },
		{ 1362, 496 },
		{ 1362, 291 },
		{ 1425, 654 },
		{ 1425, 496 },
		{ 1425, 291 },
		{ 1417, 173 },
		{ 1488, 291 },
		{ 1488, 496 },
		{ 1488, 654 },
		{ 1551, 654 },
		{ 1551, 496 },
		{ 1551, 291 },
		{ 1614, 291 },
		{ 1614, 496 },
		{ 1614, 654 },
		{ 1732, 189 },
		{ 1811, 1276 },
		{ 1843, 1213 },
		{ 1913, 630 },
		{ 1921, 732 },
		{ 2087, 370 },
		{ 2118, 1276 },
		{ 2150, 1213 },
		{ 2189, 205 },
		{ 2220, 189 },
		{ 2220, 630 },
		{ 2228, 732 },
		{ 2244, 142 },
		{ 2276, 315 },
		{ 2276, 496 },
		{ 2276, 654 },
		{ 2315, 654 },
		{ 2315, 496 },
		{ 2315, 315 },
		{ 2331, 142 },
		{ 2346, 315 },
		{ 2346, 496 },
		{ 2346, 654 },
		{ 2362, 142 },
		{ 2402, 157 },
		{ 2402, 220 },
		{ 2480, 142 },
		{ 2496, 370 },
		{ 2528, 1276 },
		{ 2559, 1213 },
		{ 2630, 630 },
		{ 2638, 732 },
		{ 2756, 69 },
		{ 2787, 69 },
		{ 2803, 370 },
		{ 2835, 1276 },
		{ 2866, 1213 },
		{ 2906, 69 },
		{ 2937, 69 },
		{ 2937, 630 },
		{ 2945, 732 },
		{ 3016, 1276 },
		{ 3055, 69 },
		{ 3087, 69 },
		{ 606, 220 },
		{ 1165, 370 },
		{ 1780, 370 }
	};
};

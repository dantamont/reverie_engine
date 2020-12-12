# Online Python compiler (interpreter) to run Python online.
# Write Python 3 code in this online editor and run it.

class Cell:
    def __init__(self):
        self.minPoint = [1, 2, 3]
        self.maxPoint = [5, 6, 7]

def sq_dist_1(point, tile, currentCell):
    sqDist = 0.0;
    for i in range(0, 3):
        v = point[i]
        if v < currentCell.minPoint[i]:
            sqDist += (currentCell.minPoint[i] - v) * (currentCell.minPoint[i] - v)
            
        if v > currentCell.maxPoint[i]:
            sqDist += (v - currentCell.maxPoint[i]) * (v - currentCell.maxPoint[i])
    print(f"{sqDist}")
    return sqDist
    
def less_than(p1, p2):
    p_out = [0, 0, 0]
    count = 0
    for x, y in zip(p1, p2):
        if x < y:
            p_out[count] = 1
        count += 1
    return p_out
    
def greater_than(p1, p2):
    p_out = [0, 0, 0]
    count = 0
    for x, y in zip(p1, p2):
        if x > y:
            p_out[count] = 1
        count += 1
    return p_out
            
def list_subtract(a, b):
    return [a[i] - b[i] for i in range(len(a))]
    
def list_el_multiply(a, b):
    return [a[i] * b[i] for i in range(len(a))]
    
def sq_dist_2(point, tile, currentCell):
    sqDist = 0.0;
    minPoint = currentCell.minPoint
    maxPoint = currentCell.maxPoint
    t1 = less_than(point, minPoint)
    t2 = greater_than(point, maxPoint)
    print(f"t1 {t1}")
    print(f"t2 {t2}")
    print(f"point {point}")
    print(f"maxPoint {maxPoint}")
    part1 = list_el_multiply(t1, list_subtract(minPoint, point))
    part2 = list_subtract(minPoint, point)
    part3 = list_el_multiply(t2, list_subtract(maxPoint, point))
    part4 = list_subtract(maxPoint, point)
    sqDist = list_el_multiply(part1, part2) + list_el_multiply(part3, part4)
    dist = sqDist[0] + sqDist[1] + sqDist[2];
    print(f"{sqDist}")
    print(f"{dist}")
    return dist

sq_dist_1([1.2, 3.4, 2.1], 1, Cell())
sq_dist_2([1.2, 3.4, 2.1], 1, Cell())



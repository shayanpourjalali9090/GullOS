def image_to_array(filename):
    with open(filename, 'rb') as f:
        header = f.read(54)
        
        # Get width and height from header
        width = int.from_bytes(header[18:22], 'little')
        height = int.from_bytes(header[22:26], 'little')
        
        data = f.read()
        
        # Read all pixels first
        pixels = []
        for i in range(0, len(data), 4):
            b = data[i]
            g = data[i+1]
            r = data[i+2]
            a = data[i+3] if i+3 < len(data) else 255
            
            if a == 0:
                pixels.append(-1)
            else:
                color = (a << 24) | (r << 16) | (g << 8) | b
                pixels.append(color)
        
        # Flip rows
        result = []
        for row in range(height-1, -1, -1):
            for col in range(width):
                result.append(pixels[row * width + col])
        
        return result

# Use it
array = image_to_array('gull.bmp')
print(array)

from PIL import Image

def convert_bmp_format(input_path, output_path):
    # Open the source BMP file
    with Image.open(input_path) as img:
        # Ensure the image is in 24-bit RGB mode
        img = img.convert("RGB")
        
        # Save the image in BMP format with the Windows 3.x header (bit offset 54)
        img.save(output_path, format='BMP', bits=24)

# Example usage
convert_bmp_format('gokujpg', 'goku2.bmp')
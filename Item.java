import java.util.Arrays; /*Included in order to allow the use of the command Arrays.asList().contains() which easily
                          allows a check to be made as to whether a value is present in an array

/**
 * Created by locw on 02/11/16.
 */
public class Item {
    private String name;
    private double price;
    private int stock;

    /*
    Below are getters and setter of the class to return and assign attribute
    values.
     */

    public String getName()
    {
        return name;
    }

    public void setName(String name)
    {
        this.name = name;
    }

    public double getPrice()
    {
        return price;
    }

    public void setPrice(double price)
    {
        this.price = price;
    }

    public int getStock()
    {
        return stock;
    }

    public void setStock(int stock)
    {
        this.stock = stock;
    }

    /*
    Allows the details of an item to be printed
     */
    public void printDetails() {
        System.out.println("Name: " + this.getName());
        System.out.println("Price: " + this.getPrice());
        System.out.println("Stock: " + this.getStock());
    }

    /*
    If the item is in the list of items in a shop, removes the item and updates
    the item list to fill in the gaps left by a null element. Otherwise prints
    an error message to the user.
     */
    public void remove(Shop shop) {
        int location = 0;

        if (Arrays.asList(shop.getItems()).contains(this)) {
            for (int i = 0; i < shop.getItems().length; i++) {
                if (shop.getItems()[i] == this) {
                    shop.getItems()[i] = null;
                    location = i;
                }
            }

            for (int i = location; i < (shop.getItems().length - 1); i++) {
                shop.getItems()[i] = shop.getItems()[i + 1];
                shop.getItems()[i + 1] = null;
            }
        } else {
            System.out.println("Not stocked");
        }
    }
}

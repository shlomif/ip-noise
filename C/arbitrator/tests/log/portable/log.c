static const double e = 2.718281828459045090795598298427648842334747314453125;

static const double e_reciprocal = 0.367879441171442334024277442949824035167694091796875;

double mylog(double number)
{
    double expr, expr_squared, x_to_the_power_of_n, result;
    int sign_rev = 0;
    int a;
    int exp_base = 0;
    
    if (number <= 0)
    {
        /* Return -google. Close to -inf. */
        return -1e100;
    }
    
    if (number < 1)
    {
        while (number < 0.5)
        {
            number *= e;
            exp_base--;
        }
    }
    else
    {
        while (number > 2)
        {
            number *= e_reciprocal;
            exp_base++;
        }
    }

    if (number == 1)
    {
        return exp_base;
    }
    else if (number < 1)
    {
        number = 1/number;
        sign_rev = 1;
    }

    expr = ((number-1)/(number+1));
    expr_squared = expr*expr;

    x_to_the_power_of_n = 2*expr;

    result = 0;

    for(a = 1 ; a <= 29 ; a++, a++)
    {
        result += x_to_the_power_of_n/a;
        x_to_the_power_of_n *= expr_squared;
    }

    if (sign_rev)
    {
        result = -result;
    }

    return result + exp_base;
}

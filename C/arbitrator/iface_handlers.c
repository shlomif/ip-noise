static int ip_noise_arbitrator_iface_handler_new_chain(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_new_state(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_move_probs(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_source(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_dest(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_protocol(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_tos(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_length_min(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_length_max(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_which_length(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_drop_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_delay_type(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_split_linear_points(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_lambda(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_time_factor(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_set_stable_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_clear_all(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);
static int ip_noise_arbitrator_iface_handler_end_connection(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params);



#define NUM_OPERATIONS 18
static operation_t operations[NUM_OPERATIONS] = 
{
	{
		0x0,
		1,
		{PARAM_TYPE_STRING, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		1,
		{PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_new_chain
	},
	{
		0x1,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STRING, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		1,
		{PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_new_state
	},
	{
		0x2,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_INT, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_move_probs
	},
	{
		0x3,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_IP_FILTER, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_source
	},
	{
		0x4,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_IP_FILTER, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_dest
	},
	{
		0x5,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_BOOL, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_protocol
	},
	{
		0x6,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_BOOL, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_tos
	},
	{
		0x8,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_length_min
	},
	{
		0x9,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_INT, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_length_max
	},
	{
		0xA,
		2,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_WHICH_PACKET_LENGTH, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_which_length
	},
	{
		0xE,
		4,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_PROB, PARAM_TYPE_PROB},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_drop_delay_prob
	},
	{
		0xF,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_DELAY_FUNCTION_TYPE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_delay_type
	},
	{
		0x10,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_SPLIT_LINEAR_POINTS, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_split_linear_points
	},
	{
		0x11,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_LAMBDA, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_lambda
	},
	{
		0x13,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_DELAY_TYPE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_time_factor
	},
	{
		0x14,
		3,
		{PARAM_TYPE_CHAIN, PARAM_TYPE_STATE, PARAM_TYPE_PROB, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_set_stable_delay_prob
	},
	{
		0x19,
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_clear_all
	},
	{
		0x10000,
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		0,
		{PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE, PARAM_TYPE_NONE},
		ip_noise_arbitrator_iface_handler_end_connection
	},
};


static int ip_noise_arbitrator_iface_handler_new_chain(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	char * name = params[0].string;
    int index;

	/* FILL IN WITH BODY*/
    ip_noise_arbitrator_data_t * data;

    printf("New Chain: \"%s\"!\n", name);

    data = self->data;

    if (data->num_chains == data->max_num_chains)
    {
        data->max_num_chains += 16;

        data->chains = realloc(data->chains, sizeof(data->chains[0])*data->max_num_chains);        
    }

    index = data->num_chains;
    data->chains[index] = chain_alloc(name);
    
    ip_noise_str2int_dict_add(data->chain_names, name, index);
    
    self->last_chain = index;

    self->last_state = -1;
    
    data->num_chains++;

    out_params[1]._int = index;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_new_state(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	char * state_name = params[1].string;

    int a;
    int index;

    ip_noise_arbitrator_data_t * data = self->data;

	/* FILL IN WITH BODY*/

    ip_noise_chain_t * chain = data->chains[chain_index];

    printf("New state: \"%s\"!\n", state_name);
    
    for(a=0;a<chain->num_states;a++)
    {
        realloc(chain->states[a]->move_tos, sizeof(chain->states[a]->move_tos[0])*(chain->num_states+1));
        chain->states[a]->move_tos[chain->num_states].comulative_prob = 0; 
    }

    if (chain->num_states == chain->max_num_states)
    {
        chain->max_num_states += 16;
        chain->states = realloc(
                chain->states,
                sizeof(chain->states[0])*chain->max_num_states
                );
    }

    index = chain->num_states;

    chain->states[index] = state_alloc(state_name);
    
    ip_noise_str2int_dict_add(chain->state_names, state_name, index);

    self->last_state = index;
    
    chain->states[index]->move_tos = malloc(sizeof(chain->states[index]->move_tos[0])*(index+1));
    for(a=0;a<index;a++)
    {
        chain->states[index]->move_tos[a].comulative_prob = 0;
    }
    chain->states[index]->move_tos[a].comulative_prob = 1;

    chain->num_states++;
    
	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_move_probs(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int num_sources = params[1]._int;
	int num_dests = params[2]._int;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_source(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	ip_noise_ip_spec_t * source = params[1].ip_filter;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_dest(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	ip_noise_ip_spec_t * dest = params[1].ip_filter;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_protocol(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int index = params[1]._int;
	int enable_or_disable = params[2].bool;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_tos(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int index = params[1]._int;
	int enable_or_disable = params[2].bool;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_length_min(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int min = params[1]._int;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_length_max(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int max = params[1]._int;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_which_length(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int which = params[1].which_packet_length;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_drop_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	ip_noise_prob_t drop_prob = params[2].prob;
	ip_noise_prob_t delay_prob = params[3].prob;

	/* FILL IN WITH BODY*/

    ip_noise_state_t * state;

    printf("Set Drop/Delay Probs - (%i,%i)!\n", chain_index, state_index);

    state = get_state(self, chain_index, state_index);

    if (state == NULL)
    {
        return IP_NOISE_RET_VALUE_INDEX_OUT_OF_RANGE;
    }

    state->drop_prob = drop_prob;
    state->delay_prob = delay_prob;

	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_delay_type(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	int delay_type = params[2].delay_function_type;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_split_linear_points(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	ip_noise_split_linear_function_t points = params[2].split_linear_points;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_lambda(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	int lambda = params[2].lambda;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_time_factor(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	int time_factor = params[2].delay_type;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_set_stable_delay_prob(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	int chain_index = params[0].chain;
	int state_index = params[1].state;
	ip_noise_prob_t stable_delay_prob = params[2].prob;



	/* FILL IN WITH BODY*/


	return 0;
}



static int ip_noise_arbitrator_iface_handler_clear_all(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{
	/* FILL IN WITH BODY*/

    ip_noise_arbitrator_data_t * data = self->data;

    int a;

    printf("Clear All!\n");

    
    for(a=0;a<data->num_chains;a++)
    {
        chain_free(data->chains[a]);
    }

    data->num_chains = 0;

    ip_noise_str2int_dict_reset(data->chain_names);

	return 0;
}



static int ip_noise_arbitrator_iface_handler_end_connection(ip_noise_arbitrator_iface_t * self, param_t * params, param_t * out_params)
{



	/* FILL IN WITH BODY*/


	return 0;
}



